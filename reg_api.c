/*
 * dma_api.c
 *
 *  Created on: 2020��10��16��
 *      Author: Administrator
 */

#ifndef SRC_DMA_API_C_
#define SRC_DMA_API_C_

#include "reg_api.h"

 u32 single_reg_read(u32 reg_address)
 {
	 u32 reg_data;
//	 reg_data=  XGpio_DiscreteRead(&Gpio, 1 );
	 reg_data = Xil_In32(reg_address);
	 return  reg_data;
 }

 void single_reg_write(u32 reg_address , u32 reg_data)
 {
	 //XGpio_DiscreteWrite(&Gpio, 1, led_state);
	 Xil_Out32(reg_address, reg_data);

 }


#endif /* SRC_DMA_API_C_ */

