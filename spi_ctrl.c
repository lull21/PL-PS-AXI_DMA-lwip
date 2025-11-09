/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include "spi_ctrl.h"

static XSpiPs_Config 	*spi_config;
static XSpiPs			spi_instance, spi_instance_id0, spi_instance_id1;


spi_device lmk2594_spi = {
	XPAR_PSU_SPI_1_BASEADDR,	// base_address
	XPAR_PSU_SPI_1_DEVICE_ID,	// device_id
	2, 							// chip_select
	0, 							// cpha
	0, 							// cpol
	0							// id_no
};

spi_device lmk01020B_spi = {
	XPAR_PSU_SPI_0_BASEADDR, 	// base_address
	XPAR_PSU_SPI_0_DEVICE_ID, 	// device_id
	2, 							// chip_select
	0, 							// cpha
	0, 							// cpol
	0							// id_no
};

spi_device lmk01020_spi = {
	XPAR_PSU_SPI_0_BASEADDR, 	// base_address
	XPAR_PSU_SPI_0_DEVICE_ID, 	// device_id
	1, 							// chip_select
	0, 							// cpha
	0, 							// cpol
	0							// id_no
};


uint32_t lmk01010_init_data[] = {
	0x80000100,
	0x00010100,
	0x00010101,
	0x00010102,
	0x00010103,
	0x00010104,
	0x00010105,
	0x00010106,
//	0x00010107,
	0x00030A07,
	0x00022A09,
	0x6800000E 	//clkin0
//	0x4800000E  //clkin1
};

uint32_t lmk01010_2594divinit_data[] = {
	0x80000100,
	0x00010100,
	0x00010101,
	0x00010102,
	0x00010103,
	0x00010104,
	0x00010105,
	0x00032006,
//	0x00032007,
	0x00030207,
	0x00022A09,
	0x6800000E  //clkin0
//	0x4800000E  //clkin1
};

//ref10m
uint32_t lmk2594_init_data[] = {//clkin1
		0x700000,
		0x6F0000,
		0x6E0000,
		0x6D0000,
		0x6C0000,
		0x6B0000,
		0x6A0000,
		0x690021,
		0x680000,
		0x670000,
		0x663EC0,
		0x650011,
		0x640000,
		0x630000,
		0x620500,
		0x610888,
		0x600000,
		0x5F0000,
		0x5E0000,
		0x5D0000,
		0x5C0000,
		0x5B0000,
		0x5A0000,
		0x590000,
		0x580000,
		0x570000,
		0x560000,
		0x552080,
		0x540000,
		0x530000,
		0x52DC00,
		0x510000,
		0x500000,
		0x4F0060,
		0x4E0053,
		0x4D0000,
		0x4C000C,
		0x4B0980,
		0x4A0000,
		0x49003F,
		0x480001,
		0x470081,
		0x46C350,
		0x450000,
		0x4403E8,
		0x430000,
		0x4201F4,
		0x410000,
		0x401388,
		0x3F0000,
		0x3E0322,
		0x3D00A8,
		0x3C0000,
		0x3B0001,
		0x3A1001,
		0x390020,
		0x380000,
		0x370000,
		0x360000,
		0x350000,
		0x340820,
		0x330080,
		0x320000,
		0x314180,
		0x300300,
		0x2F0300,
		0x2E07FC,
		0x2DC0FF,
		0x2C3F22,
		0x2B0000,
		0x2A0000,
		0x290000,
		0x280000,
		0x270001,
		0x260000,
		0x250204,
		0x2400EA,
		0x230004,
		0x220000,
		0x211E21,
		0x200393,
		0x1F43EC,
		0x1E318C,
		0x1D318C,
		0x1C0488,
		0x1B0002,
		0x1A0DB0,
		0x190C2B,
		0x18071A,
		0x17007C,
		0x160001,
		0x150401,
		0x14D048,
		0x1327B7,
		0x120064,
		0x110109,
		0x100080,
		0x0F064F,
		0x0E1E70,
		0x0D4000,
		0x0C5001,
		0x0B0038,
		0x0A10D8,
		0x090604,
		0x082000,
		0x0740B2,
		0x06C802,
		0x0500C8,
		0x040A43,
		0x030642,
		0x020500,
		0x010808,
		0x00641C


//		0x700000,//2594
//		0x6F0000,
//		0x6E0000,
//		0x6D0000,
//		0x6C0000,
//		0x6B0000,
//		0x6A0000,
//		0x690021,
//		0x680000,
//		0x670000,
//		0x660000,
//		0x650011,
//		0x640000,
//		0x630000,
//		0x620000,
//		0x610888,
//		0x600000,
//		0x5F0000,
//		0x5E0000,
//		0x5D0000,
//		0x5C0000,
//		0x5B0000,
//		0x5A0000,
//		0x590000,
//		0x580000,
//		0x570000,
//		0x560000,
//		0x550000,
//		0x540000,
//		0x530000,
//		0x520000,
//		0x510000,
//		0x500000,
//		0x4F0000,
//		0x4E0003,
//		0x4D0000,
//		0x4C000C,
//		0x4B0940,
//		0x4A0000,
//		0x49003F,
//		0x480001,
//		0x470081,
//		0x46C350,
//		0x450000,
//		0x4403E8,
//		0x430000,
//		0x4201F4,
//		0x410000,
//		0x401388,
//		0x3F0000,
//		0x3E0322,
//		0x3D00A8,
//		0x3C0000,
//		0x3B0001,
//		0x3A9001,
//		0x390020,
//		0x380000,
//		0x370000,
//		0x360000,
//		0x350000,
//		0x340820,
//		0x330080,
//		0x320000,
//		0x314180,
//		0x300300,
//		0x2F0300,
//		0x2E07FC,
//		0x2DC0DF,
//		0x2C1F23,
//		0x2B0000,
//		0x2A0000,
//		0x290000,
//		0x280000,
//		0x2703E8,
//		0x260000,
//		0x250304,
//		0x24A000,
//		0x230004,
//		0x220000,
//		0x211E21,
//		0x200393,
//		0x1F43EC,
//		0x1E318C,
//		0x1D318C,
//		0x1C0488,
//		0x1B0002,
//		0x1A0DB0,
//		0x190C2B,
//		0x18071A,
//		0x17007C,
//		0x160001,
//		0x150401,
//		0x14E048,
//		0x1327B7,
//		0x120064,
//		0x11012C,
//		0x100080,
//		0x0F064F,
//		0x0E1E70,
//		0x0D4000,
//		0x0C5050,
//		0x0B0028,
//		0x0A10D8,
//		0x090604,
//		0x082000,
//		0x0740B2,
//		0x06C802,
//		0x0500C8,
//		0x040A43,
//		0x030642,
//		0x020500,
//		0x010808,
//		0x00251C

//		0x7D2288,//2572
//		0x7C0000,
//		0x7B0000,
//		0x7A0000,
//		0x790000,
//		0x780000,
//		0x770000,
//		0x760000,
//		0x750000,
//		0x740000,
//		0x730000,
//		0x727802,
//		0x710000,
//		0x700000,
//		0x6F0000,
//		0x6E0000,
//		0x6D0000,
//		0x6C0000,
//		0x6B0000,
//		0x6A0007,
//		0x694440,
//		0x680000,
//		0x670000,
//		0x660000,
//		0x650000,
//		0x640000,
//		0x630000,
//		0x620000,
//		0x610000,
//		0x600000,
//		0x5F0000,
//		0x5E0000,
//		0x5D0000,
//		0x5C0000,
//		0x5B0000,
//		0x5A0000,
//		0x590000,
//		0x580000,
//		0x570000,
//		0x560000,
//		0x550000,
//		0x540000,
//		0x530000,
//		0x520000,
//		0x510000,
//		0x500000,
//		0x4F0000,
//		0x4E0001,
//		0x4D0000,
//		0x4C000C,
//		0x4B0940,
//		0x4A0000,
//		0x49003F,
//		0x480001,
//		0x470081,
//		0x46C350,
//		0x450000,
//		0x4403E8,
//		0x430000,
//		0x4201F4,
//		0x410000,
//		0x401388,
//		0x3F0000,
//		0x3E00AF,
//		0x3D00A8,
//		0x3C03E8,
//		0x3B0001,
//		0x3A9001,
//		0x390020,
//		0x380000,
//		0x370000,
//		0x360000,
//		0x350000,
//		0x340421,
//		0x330080,
//		0x320080,
//		0x314180,
//		0x3003E0,
//		0x2F0300,
//		0x2E07F0,
//		0x2DC61F,
//		0x2C1F23,
//		0x2B014D,
//		0x2A0000,
//		0x290000,
//		0x280000,
//		0x2703E7,
//		0x260000,
//		0x250305,
//		0x2400D0,
//		0x230004,
//		0x220010,
//		0x211E01,
//		0x2005BF,
//		0x1FC3E6,
//		0x1E18A6,
//		0x1D0000,
//		0x1C0488,
//		0x1B0002,
//		0x1A0808,
//		0x190624,
//		0x18071A,
//		0x17007C,
//		0x160001,
//		0x150409,
//		0x144848,
//		0x1327B7,
//		0x120064,
//		0x110096,
//		0x100080,
//		0x0F060E,
//		0x0E1820,
//		0x0D4000,
//		0x0C5001,
//		0x0BB018,
//		0x0A10F8,
//		0x090004,
//		0x082000,
//		0x0700B2,
//		0x06C802,
//		0x0530C8,
//		0x040A43,
//		0x030782,
//		0x020500,
//		0x010808,
//		0x00211C

};
//ref122.88
//uint32_t lmk2594_init_data[] = {//clkin1
//
//		0x0000249E,
//		0x4D0000,
//		0x4C000C,
//		0x4B0940,
//		0x4A0000,
//		0x49003F,
//		0x48000E,
//		0x470081,
//		0x46C350,
//		0x450000,
//		0x4403E8,
//		0x430000,
//		0x4201F4,
//		0x410000,
//		0x401388,
//		0x3F0000,
//		0x3E0322,
//		0x3D00A8,
//		0x3C0000,
//		0x3B0001,
//		0x3A8001,
//		0x390020,
//		0x380000,
//		0x370000,
//		0x360000,
//		0x350000,
//		0x340820,
//		0x330080,
//		0x320000,
//		0x314180,
//		0x300300,
//		0x2F0300,
//		0x2E07FC,
//		0x2DC0CC,
//		0x2C0C23,
//		0x2B0000,
//		0x2A0000,
//		0x290000,
//		0x280000,
//		0x270014,
//		0x260000,
//		0x250304,
//		0x240C80,
//		0x230004,
//		0x220000,
//		0x211E21,
//		0x200393,
//		0x1F43EC,
//		0x1E318C,
//		0x1D318C,
//		0x1C0488,
//		0x1B0002,
//		0x1A0DB0,
//		0x190624,
//		0x18071A,
//		0x17007C,
//		0x160001,
//		0x150401,
//		0x14C848,
//		0x1327B7,
//		0x120064,
//		0x110117,
//		0x100080,
//		0x0F064F,
//		0x0E1E70,
//		0x0D4000,
//		0x0C5032,
//		0x0B0018,
//		0x0A10D8,
//		0x090604,
//		0x082000,
//		0x0740B2,
//		0x06C802,
//		0x0500C8,
//		0x040B43,
//		0x030642,
//		0x020500,
//		0x010808,
//
//		0x0000249C
//};

int32_t spi_write_and_read(spi_device *dev, uint8_t *data, uint8_t bytes_number)
{
	uint32_t initss;
	spi_instance = dev->device_id == 0 ? spi_instance_id0 : spi_instance_id1;
	initss = XSpiPs_ReadReg(dev->base_address, XSPIPS_CR_OFFSET);
	initss = initss & (uint32_t)(~XSPIPS_CR_SSCTRL_MASK);
	initss = initss | (0x7 << XSPIPS_CR_SSCTRL_SHIFT);
	XSpiPs_WriteReg(dev->base_address, XSPIPS_CR_OFFSET, initss);

	XSpiPs_SetOptions(&spi_instance, XSPIPS_MASTER_OPTION |
			XSPIPS_DECODE_SSELECT_OPTION | XSPIPS_FORCE_SSELECT_OPTION |
			((dev->cpol == 1) ? XSPIPS_CLK_ACTIVE_LOW_OPTION : 0) |
			((dev->cpha == 1) ? XSPIPS_CLK_PHASE_1_OPTION : 0));

	XSpiPs_SetSlaveSelect(&spi_instance, (uint8_t) 0x7);

	XSpiPs_SetClkPrescaler(&spi_instance, XSPIPS_CLK_PRESCALE_64);

	XSpiPs_SetSlaveSelect(&spi_instance,  (uint8_t) dev->chip_select);

	XSpiPs_PolledTransfer(&spi_instance, data, data, bytes_number);
	XSpiPs_SetSlaveSelect(&spi_instance,  (uint8_t) 0x7);

	return 0;
}

int32_t spi_write32(spi_device *spi, uint32_t val) {
	uint8_t buf[4];
	int32_t ret;
	uint32_t cmd = val;

	buf[0] = cmd >> 24;
	buf[1] = cmd >> 16;
	buf[2] = cmd >> 8;
	buf[3] = cmd & 0xFF;

	ret = spi_write_and_read(spi, buf, 4);
	return ret;
}

int32_t spi_write24(spi_device *spi, uint32_t val) {
	uint8_t buf[2];
	int32_t ret;
	uint32_t cmd = val;

	buf[0] = cmd >> 16;
	buf[1] = cmd >> 8;
	buf[2] = cmd & 0xFF;

	ret = spi_write_and_read(spi, buf, 3);
	return ret;
}

int32_t lmk01010_init(spi_device *spi) {
	int ret;
	int i;

	for (i = 0; i < ARRAY_SIZE(lmk01010_init_data); i++) {
		ret = spi_write32(spi, lmk01010_init_data[i]);
	}
	return ret;
}

int32_t lmk01010_2594divinit(spi_device *spi) {
	int ret;
	int i;

	for (i = 0; i < ARRAY_SIZE(lmk01010_2594divinit_data); i++) {
		ret = spi_write32(spi, lmk01010_2594divinit_data[i]);
	}
	return ret;
}

int32_t lmk2594_init(spi_device *spi) {
	int ret;
	int i;

	ret = spi_write24(spi, lmk2594_init_data[0]);
	sleep(0.3);
	for (i = 1; i < ARRAY_SIZE(lmk2594_init_data); i++) {
		ret = spi_write24(spi, lmk2594_init_data[i]);
	}
	return ret;
}

int32_t spi_init(uint16_t device_id, XSpiPs *spi_instance)
{
	#ifdef _XPARAMETERS_PS_H_
	spi_config = XSpiPs_LookupConfig(device_id);
	if (spi_config == NULL)
		return -1;

	if (XSpiPs_CfgInitialize(spi_instance, spi_config, spi_config->BaseAddress) != 0)
		return -1;
	#endif
	return 0;
}

int32_t platform_init(void)
{
	spi_init(XPAR_PSU_SPI_0_DEVICE_ID, &spi_instance_id0);
	spi_init(XPAR_PSU_SPI_1_DEVICE_ID, &spi_instance_id1);

	return 0;
}
