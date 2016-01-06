#ifndef __I2C_H__
#define __I2C_H__

#include <stdint.h>
// Define signal value.
#define SIG_I2C_RX_COMPLETE 0x02
#define SIG_I2C_TX_COMPLETE 0x03

#define I2C_RXTXBUFFER_SIZE 100
typedef struct{
	uint8_t data[I2C_RXTXBUFFER_SIZE];
	int len;
	int index;
	int addr;
}I2cRxTxBufferDef;

void I2C_init(void);

uint8_t I2C_read(int addr, int reg);
void I2C_readMulti(int addr, int reg, uint8_t* data, int bytesCount);
void I2C_write(int addr, int reg, int data);
int32_t getTime(void);

#endif
