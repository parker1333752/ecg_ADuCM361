#ifndef __ADXL346_H__
#define __ADXL346_H__

#include "ADUCM360.h"
#include "DioLib.h"
#include "SpiLib.h"

#define SPIx 		pADI_SPI0


// Register definition.
#define DEVID		0x00
#define DATAX0      0x32
#define DATAX1      0x33
#define DATAY0      0x34
#define DATAY1      0x35
#define DATAZ0      0x36
#define DATAZ1      0x37
#define FIFO_CTL    0x38
#define FIFO_STATUS 0x39

void ADXL346_init(void);
uint8_t ADXL_read_reg(uint8_t reg);
void ADXL_write_reg(uint8_t reg, uint8_t data);

int16_t ADXL_read_datax(void);
int16_t ADXL_read_datay(void);
int16_t ADXL_read_dataz(void);
uint8_t ADXL_set_fifo(uint8_t conf);
uint8_t ADXL_get_fifocount(void);
uint8_t ADXL_get_fifotrig(void);

#endif
