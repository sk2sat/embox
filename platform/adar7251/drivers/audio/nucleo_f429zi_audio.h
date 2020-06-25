/*
 * nucleo_f429zi_audio.h
 *
 *  Created on: Jun 1, 2020
 *      Author: anton
 */

#ifndef PLATFORM_ADAR7251_DRIVERS_AUDIO_NUCLEO_F429ZI_AUDIO_H_
#define PLATFORM_ADAR7251_DRIVERS_AUDIO_NUCLEO_F429ZI_AUDIO_H_

#include <stdint.h>

struct sai_device;

extern struct sai_device *sai_init(void);

extern void sai_start(struct sai_device *sai_dev, int channels);

extern void sai_stop(struct sai_device *sai_dev);

extern int sai_receive(struct sai_device *sai_dev, uint8_t *buf, int len);

#define SAI_SAMPLES_BUFFER 0x200

#endif /* PLATFORM_ADAR7251_DRIVERS_AUDIO_NUCLEO_F429ZI_AUDIO_H_ */
