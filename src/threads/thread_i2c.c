#include "thread_i2c.h"
#include "mpu6050.h"
#include "I2cLib.h"
#include "thread_uart.h"

extern volatile uint8_t i2c_rx_data;
extern I2cRxTxBufferDef txbuffer;
extern I2cRxTxBufferDef rxbuffer;
osThreadId tid_Thread_i2c;

#define MPU6050_ADDR	0xd0

volatile int init_result;
osEvent os_result;
TM_MPU6050_t data;
int32_t time;

#pragma pack(push, 1)
typedef struct{
	int16_t Accelerometer_X; /*!< Accelerometer value X axis */
	int16_t Accelerometer_Y; /*!< Accelerometer value Y axis */
	int16_t Accelerometer_Z; /*!< Accelerometer value Z axis */
	int16_t Gyroscope_X;     /*!< Gyroscope value X axis */
	int16_t Gyroscope_Y;     /*!< Gyroscope value Y axis */
	int16_t Gyroscope_Z;     /*!< Gyroscope value Z axis */
	int16_t date;
}MpuDataDef;
#pragma pack(pop)
MpuDataDef mpu_data;

void Thread_i2c (void const *argument) {
	/* initialize */
	if(TM_MPU6050_Result_Ok != TM_MPU6050_Init(&data,TM_MPU6050_Device_0,TM_MPU6050_Accelerometer_8G,TM_MPU6050_Gyroscope_250s)){
		// TODO: Change to periodically check device.
		return;
	}
	
	while(1) {
//		if(TM_MPU6050_ReadSta(&data, DATA_RDY_INT)){
			osDelay(2);
			TM_MPU6050_ReadAll(&data);
			mpu_data.Accelerometer_X = data.Accelerometer_X;
			mpu_data.Accelerometer_Y = data.Accelerometer_Y;
			mpu_data.Accelerometer_Z = data.Accelerometer_Z;
			mpu_data.Gyroscope_X = data.Gyroscope_X;
			mpu_data.Gyroscope_Y = data.Gyroscope_Y;
			mpu_data.Gyroscope_Z = data.Gyroscope_Z;
			mpu_data.date = data.Time & 0x7fff; 
//			UART_Write_Frame(0x02, sizeof(MpuDataDef), &mpu_data);
//		}
	}
}

void I2C0_Master_Int_Handler(void)
{
	volatile int uiStatus = I2cSta(0);
	if((uiStatus & I2CMSTA_RXREQ) == I2CMSTA_RXREQ) {/* i2c receive interrupt */
		rxbuffer.data[rxbuffer.index++] = I2cRx(0);
		if(rxbuffer.index == rxbuffer.len)osSignalSet(tid_Thread_i2c, SIG_I2C_RX_COMPLETE);
	}
	if((uiStatus & I2CMSTA_TXREQ) == I2CMSTA_TXREQ) {// Master Transmit IRQ 
		if (txbuffer.index < txbuffer.len ) {
			I2cTx(0, txbuffer.data[txbuffer.index++]);
		} else { // TXREQ disabled to avoid multiple unecessary interrupts 
			I2cMCfg(I2CMCON_TXDMA_DIS|I2CMCON_RXDMA_DIS, I2CMCON_IENCMP|I2CMCON_IENRX|I2CMCON_IENTX_DIS, I2CMCON_MAS_EN);
		}
	}
	if((uiStatus & I2CMSTA_TCOMP_SET) == I2CMSTA_TCOMP_SET) {// communication complete	
		I2cMCfg(I2CMCON_TXDMA_DIS|I2CMCON_RXDMA_DIS, I2CMCON_IENCMP|I2CMCON_IENRX|I2CMCON_IENTX_EN, I2CMCON_MAS_EN);   // TXREQ enabled for future master transmissions    
		osSignalSet(tid_Thread_i2c, SIG_I2C_TX_COMPLETE);
	}
}
