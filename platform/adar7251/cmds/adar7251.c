#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include <drivers/adar7251_driver.h>
#include <drivers/nucleo_f429zi_audio.h>

#define PORT 9930
#define BUFLEN 1024

static uint8_t rx_buf[SAI_SAMPLES_BUFFER * 2]; /* half of sai buffer */
static struct adar7251_dev adar7251_dev;
static char buf[BUFLEN];

#define ADAR_PACKET_LABEL    "ADAR"
#define ADAR_CMD_SET_CONF    (0x01)
#define ADAR_CMD_GET_CONF    (0x02)
#define ADAR_CMD_GET_STATUS  (0x03)
#define ADAR_CMD_START       (0x04)

int main(int argc, char **argv) {
	int data_len;
	struct sockaddr_in si_me, si_other;
	int s, slen;
	int port;
	int rlen;

	slen = sizeof(si_other);
	port = PORT;

	if (argc > 1) {
		port = atoi(argv[1]);
	}

	s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (s == -1) {
		printf("socket() failed");
		return -1;
	}

	memset((char *) &si_me, 0, sizeof(si_me));

	si_me.sin_family = AF_INET;
	si_me.sin_port = htons(port);
	si_me.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(s, (void *) &si_me, sizeof(si_me))==-1) {
		close(s);
		return -1;
	}

	printf("starting adar7251\n");
	adar7251_hw_init(&adar7251_dev);

	printf("configure as 4 channels ADC\n");
	adar7251_prepare(&adar7251_dev, 4);

	while(1) {
		printf("waiting UDP packet on port %d\n", port);

		rlen = recvfrom(s, buf, BUFLEN, 0, (void *)&si_other, &slen) ;
		if (rlen == -1){
			printf("recvfrom() failed");
			close(s);
			return 0;
		}
		if (memcmp(buf, ADAR_PACKET_LABEL, sizeof(ADAR_PACKET_LABEL))) {
			printf("wrong label must be '%s'\n", ADAR_PACKET_LABEL);
			continue;
		}

		switch(buf[sizeof(ADAR_PACKET_LABEL)]) {
		case ADAR_CMD_SET_CONF:
			printf("configure as 4 channels ADC\n");
			adar7251_prepare(&adar7251_dev, 4);
			break;
		case ADAR_CMD_START:
		{

			adar7251_start(&adar7251_dev);

			printf("conversation loop\n");

			adar7251_frame_start(&adar7251_dev);

			while(1) {
				data_len = sai_receive(adar7251_dev.sai_dev, rx_buf, sizeof(rx_buf));
				if (data_len == 0) {
					printf("sai timeout\n");
					continue;
				}

				if (sendto(s, rx_buf, data_len, 0, (void *) &si_other, slen)==-1) {
					adar7251_stop(&adar7251_dev);
					printf("sendto() failed");
					break;
				}
			}
		}
		break;
		default:
			printf("wrong command %d", buf[sizeof(ADAR_PACKET_LABEL)]);
			break;
		}

	}

	return 0;
}
