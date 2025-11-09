/*
 * spi_crtl.h
 *
 *  Created on: 2018��1��5��
 *      Author: liche
 */

#ifndef SRC_SPI_CTRL_H_
#define SRC_SPI_CTRL_H_
#include <stdint.h>
#include <xspips.h>
#include <sleep.h>
#include "xparameters.h"

#define ARRAY_SIZE(ar)		(sizeof(ar)/sizeof(ar[0]))

typedef struct {
	uint32_t	base_address;
	uint32_t	device_id;
	uint8_t		chip_select;
	uint32_t	cpha;
	uint32_t	cpol;
	uint8_t		id_no;
} spi_device;

int32_t spi_write_and_read(spi_device *dev, uint8_t *data, uint8_t bytes_number);
int32_t spi_write24(spi_device *spi, uint32_t val);
int32_t spi_write32(spi_device *spi, uint32_t val);
int32_t lmk01010_init(spi_device *spi);
int32_t lmk01010_2594divinit(spi_device *spi);
int32_t lmk2594_init(spi_device *spi);
int32_t spi_init(uint16_t device_id, XSpiPs *spi_instance);
int32_t platform_init(void);



#endif /* SRC_SPI_CTRL_H_ */
