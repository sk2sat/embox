/*
 * adar_sai.h
 *
 *  Created on: Jun 9, 2020
 *      Author: anton
 */

#ifndef PLATFORM_ADAR7251_DRIVERS_AUDIO_ADAR_SAI_H_
#define PLATFORM_ADAR7251_DRIVERS_AUDIO_ADAR_SAI_H_

#include <stdint.h>

#include <drivers/nucleo_f429zi_audio.h>

struct thread;

struct sai_device {
	uint32_t sai_buf[SAI_SAMPLES_BUFFER]; /* 2 * 1024  udp frames */
	struct thread *sai_thread;
	int (*sai_callback)(const void *input, unsigned long frameCount, void *userData );
	void *sai_user_data;

	int volatile sai_active;
	uint8_t volatile*sai_cur_buf;
	int volatile buf_num;

	struct __SAI_HandleTypeDef *sai_hw_dev;
};

extern struct sai_device sai_device;

#endif /* PLATFORM_ADAR7251_DRIVERS_AUDIO_ADAR_SAI_H_ */
