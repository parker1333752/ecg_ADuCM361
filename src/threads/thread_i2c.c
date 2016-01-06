#include "thread_i2c.h"
#include "mpu6050.h"
#include "I2cLib.h"
#include "thread_uart.h"
#include "timer.h"

extern volatile uint8_t i2c_rx_data;
extern I2cRxTxBufferDef txbuffer;
extern I2cRxTxBufferDef rxbuffer;
osThreadId tid_Thread_i2c;

volatile int init_result;
osEvent os_result;
TM_MPU6050_t data;
MpuDataDef mpu_data;
int32_t time;

void Thread_i2c (void const *argument) {
	/* initialize */
	if(TM_MPU6050_Result_Ok != TM_MPU6050_Init(&data,TM_MPU6050_Device_0,TM_MPU6050_Accelerometer_8G,TM_MPU6050_Gyroscope_250s)){
		// TODO: Change to periodically check device.
		return;
	}
	Timer0_init(400);

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

void GP_Tmr0_Int_Handler(){
	volatile int flag = GptSta(pADI_TM0);
	if(flag & TSTA_TMOUT){
		GptClrInt(pADI_TM0, TCLRI_TMOUT);
		// mpu_data.date = getTime() & 0x7fff;
		UART_Write_Frame(0x02, sizeof(MpuDataDef), &mpu_data);
	} else if(flag & TSTA_CAP){
		GptClrInt(pADI_TM0, TCLRI_CAP);
	}
}

void I2C0_Master_Int_Handler(void)
{
	volatile int uiStatus = I2cSta(0);
	volatile uint8_t data;
	if((uiStatus & I2CMSTA_RXREQ) == I2CMSTA_RXREQ) {/* i2c receive interrupt */
		data = I2cRx(0)&0xff;
		if(rxbuffer.index < rxbuffer.len){
			rxbuffer.data[rxbuffer.index++] = data;
			if(rxbuffer.index == rxbuffer.len){
				osSignalSet(tid_Thread_i2c, SIG_I2C_RX_COMPLETE);
			}
		}
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
		if(rxbuffer.index == 0 && rxbuffer.index < rxbuffer.len){
			I2cMRdCfg(rxbuffer.addr, rxbuffer.len, I2CMRXCNT_EXTEND_DIS);
		} else {
			osSignalSet(tid_Thread_i2c, SIG_I2C_TX_COMPLETE);
		}
	}
}
