/*
 * nucleo_f429zi_audio.c
 *
 *  Created on: Jun 1, 2020
 *      Author: anton
 */
#include <util/log.h>

#include <util/math.h>

#include <unistd.h>
#include <assert.h>
#include <string.h>

#include <kernel/irq.h>
#include <kernel/thread.h>
#include <kernel/thread/thread_sched_wait.h>
#include <kernel/sched.h>

#include <stm32f4xx_hal.h>
#include <stm32f4xx_hal_dma.h>

#include <framework/mod/options.h>

#include "adar_sai.h"


#define IRQ_NUM     OPTION_GET(NUMBER,irq_num)

struct sai_device sai_device;

static SAI_HandleTypeDef hsai_BlockA1;

static irq_return_t sai_interrupt(unsigned int irq_num, void *dev_id) {
	struct sai_device *dev = dev_id;

	HAL_DMA_IRQHandler(dev->sai_hw_dev->hdmarx);

	return IRQ_HANDLED;
}

static void sai1_hw_init(SAI_HandleTypeDef *hsai, int channels) {
	hsai->Instance = SAI1_Block_A;
	hsai->Init.Protocol = SAI_FREE_PROTOCOL;
	hsai->Init.AudioMode = SAI_MODESLAVE_RX;
	switch (channels) {
	case 2:
		hsai->Init.DataSize = SAI_DATASIZE_16;
		break;
	case 4:
		hsai->Init.DataSize = SAI_DATASIZE_32;
		break;
	default:
		log_error("support only 2 or 4 channels");
		return;
	}
	hsai->Init.FirstBit = SAI_FIRSTBIT_MSB;
	hsai->Init.ClockStrobing = SAI_CLOCKSTROBING_FALLINGEDGE;
	hsai->Init.Synchro = SAI_ASYNCHRONOUS;
	hsai->Init.OutputDrive = SAI_OUTPUTDRIVE_DISABLE;
	hsai->Init.FIFOThreshold = SAI_FIFOTHRESHOLD_EMPTY;
	//hsai->Init.FIFOThreshold = SAI_FIFOTHRESHOLD_1QF;
	switch (channels) {
	case 2:
		hsai->FrameInit.FrameLength = 32;
		hsai->FrameInit.ActiveFrameLength = 16;
		break;
	case 4:
		hsai->FrameInit.FrameLength = 64;
		hsai->FrameInit.ActiveFrameLength = 32;
		break;
	default:
		log_error("support only 2 or 4 channels");
		return;
	}
	hsai->FrameInit.FSDefinition = SAI_FS_STARTFRAME;
	hsai->FrameInit.FSPolarity = SAI_FS_ACTIVE_LOW;
	hsai->FrameInit.FSOffset = SAI_FS_FIRSTBIT;
	hsai->SlotInit.FirstBitOffset = 0;
	switch (channels) {
	case 2:
		hsai->SlotInit.SlotSize = SAI_SLOTSIZE_16B;
		break;
	case 4:
		hsai->SlotInit.SlotSize = SAI_SLOTSIZE_32B;
		break;
	default:
		log_error("support only 2 or 4 channels");
		return;
	}

	hsai->SlotInit.SlotNumber = 2;
	hsai->SlotInit.SlotActive = 0x00000003;
	if (HAL_SAI_Init(&hsai_BlockA1) != HAL_OK) {
		log_error("HAL_SAI_Init faioled");
	}
}

/**
  * Enable DMA controller clock
  */
static void sai1_dma_init(void) {
	/* DMA controller clock enable */
	__HAL_RCC_DMA2_CLK_ENABLE();

	/* DMA interrupt init */
	/* DMA2_Stream1_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(DMA2_Stream1_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(DMA2_Stream1_IRQn);
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void sai_gpio_init(void) {
	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOE_CLK_ENABLE();
	__HAL_RCC_GPIOH_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
}

struct sai_device *sai_init(void) {
	int res;

	sai_device.sai_hw_dev = &hsai_BlockA1;
	sai_device.sai_active = 0;
	sai_device.sai_cur_buf = NULL;

	/* Initialize all configured peripherals */
	sai_gpio_init();
	sai1_dma_init();
	res = irq_attach(DMA2_Stream1_IRQn, sai_interrupt, 0, &sai_device, "");
	if (res < 0) {
		log_error("irq_attach failed errcode %d", res);

		return NULL;
	}

	return &sai_device;
}

void sai_start(struct sai_device *sai_dev, int channels) {
	sai1_hw_init(sai_device.sai_hw_dev, channels);

	HAL_SAI_Receive_DMA(sai_device.sai_hw_dev, (uint8_t *)&sai_device.sai_buf[0], sizeof(sai_device.sai_buf) / 4);
}

void sai_stop(struct sai_device *sai_dev) {
	HAL_SAI_DMAStop(sai_device.sai_hw_dev);
	HAL_SAI_DeInit(sai_device.sai_hw_dev);
}

int sai_receive(struct sai_device *sai_dev, uint8_t *buf, int length) {
	int res;
	int len;
	uint8_t *cur_buf;

	len = min(length, sizeof(sai_dev->sai_buf) / 2);
	sai_dev->sai_thread = thread_self();
	sai_dev->sai_cur_buf = NULL;
	sai_dev->sai_active = 1;

	res  = SCHED_WAIT_TIMEOUT(sai_dev->sai_cur_buf, 1000000);
	if (res != 0) {
		log_error("timeout");
		return 0;
	}

	cur_buf = (uint8_t *)sai_dev->sai_cur_buf;
	sai_dev->sai_cur_buf = NULL;

	memcpy(buf, cur_buf, len);

	return len;
}

STATIC_IRQ_ATTACH(IRQ_NUM, sai_interrupt, &sai_device);
