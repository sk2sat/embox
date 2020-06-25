#ifndef PLATFORM_ADAR7251_DRIVERS_ADAR7251_DRIVER_H_
#define PLATFORM_ADAR7251_DRIVERS_ADAR7251_DRIVER_H_

struct spi_device;
struct sai_device;

struct adar_config {
	int chan_num;
};

struct adar7251_dev {
	int spi_bus;
	struct spi_device *spi_dev;
	struct sai_device *sai_dev;

	struct adar_config conf;
};

extern int adar7251_hw_init(struct adar7251_dev *dev);

extern void adar7251_prepare(struct adar7251_dev *dev, int ch_num);

extern void adar7251_start(struct adar7251_dev *dev);

/* write down batch of registers */
extern void adar_preset_pll(struct adar7251_dev *dev);

/* write down batch of registers */
extern void adar_preset(struct adar7251_dev *dev, int channels);

extern void adar_scan_flash(struct adar7251_dev *dev);

#endif /* PLATFORM_ADAR7251_DRIVERS_ADAR7251_DRIVER_H_ */
