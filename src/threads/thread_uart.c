#include "thread_uart.h"
#include "uart.h"
#include "utils.h"
#include <stdio.h>
#include "pools.h"

/* Define signal numbers. */
#define SIG_UART_SEND_COMPLETE 0x55

/* Define uart write message queue. For storing data that need to send from MCU. */
#define Q_UartWrite_Size 100
osMessageQId Q_UartWrite;
osMessageQDef (Q_UartWrite, Q_UartWrite_Size, uint8_t);

/* Define uart read message queue. For storing data that MCU received. */
#define Q_UartRead_Size 100
osMessageQId Q_UartRead;
osMessageQDef (Q_UartRead, Q_UartRead_Size, uint8_t);

/* Define Pool for usage of MPU data queue. */
#define P_MPUData_Size 10
_PoolDef(P_MPUData, P_MPUData_Size, MPUDataDef);

osThreadId tid_Thread_uart;

osThreadId tid_Thread_uart_send;
osThreadDef_t ThreadDef_uart_send = {Thread_uart_send, osPriorityNormal, 1, 0};

char flagUartInitComplete = 0;

int status_machine(int status, uint8_t data, MPUDataDef **pmpu);

// Receive and process uart data.
void Thread_uart (void const *argument) {
	uint8_t data;
	osEvent os_result;
	int mpu_status = 0;
	MPUDataDef *pmpu = NULL;

	// Configure peripherals.
	UART_init();
	
	// Create message queue
	Q_UartWrite = osMessageCreate(osMessageQ(Q_UartWrite), NULL);
	Q_UartRead = osMessageCreate(osMessageQ(Q_UartRead), NULL);

	// create thread uart send
	tid_Thread_uart_send = osThreadCreate(&ThreadDef_uart_send, NULL);

	// Tell other threads that initialization complete.
	flagUartInitComplete = 1;
	
	while(1) {
		// Wait for receiving uart data.
		os_result = osMessageGet(Q_UartRead, osWaitForever);
		data = os_result.value.v;

		if(os_result.status == osEventMessage){
			// TODO: process data...
			if((mpu_status = status_machine(mpu_status, data, &pmpu)) == 0xff){
				_PoolInc(P_MPUData);
				pmpu = NULL;
				mpu_status = 0;
			}
		}
	}
}

int status_machine(int status, uint8_t data, MPUDataDef **rt_pmpu){
	static uint8_t s[10];
	static int s_top = 0;
	MPUDataDef *pmpu = *rt_pmpu;
	uint8_t sum;
	if(status == 0){ // idle
		if(data == 0x55)status = 1;
	}else if(status == 1){ // received first byte;
		if(data == 0x51){
			s_top = 0;status = 2; // acc
		}else if(data == 0x52){
			s_top = 0;status = 3; // omega
		}else if(data == 0x53){
			s_top = 0;status = 4; // angle
		}else{ status = 0; } // error status.
	}else if(status == 2 || status == 3 || status == 4){ // receive data
		if(s_top < 9){
			s[s_top++] = data;
		}else status += 100;
	}else if(status > 100){
		status = status - 101 + 0x50;
		sum = 0xff & status;
		sum += 0x55;
		sum -= s[--s_top];
		while(s_top > 0)sum += s[--s_top];
		if(sum != 0)status = 0; // sum check failed;
		else{
			if(pmpu == NULL)pmpu = _PoolAlloc(P_MPUData);
			if(status == 0x51){ // acc
				pmpu->accx = *(int16_t*)(s+0);
				pmpu->accy = *(int16_t*)(s+2);
				pmpu->accz = *(int16_t*)(s+4);
				status = 0;
			}else if(status == 0x52){ // omega
				pmpu->omegax = *(int16_t*)(s+0);
				pmpu->omegay = *(int16_t*)(s+2);
				pmpu->omegaz = *(int16_t*)(s+4);
				status = 0;
			}else if(status == 0x53){ // angle
				pmpu->anglex = *(int16_t*)(s+0);
				pmpu->angley = *(int16_t*)(s+2);
				pmpu->anglez = *(int16_t*)(s+4);
				status = 0xff;
			}else status = 0; // status error.
		}
	}
	*rt_pmpu = pmpu;
	return status;
}

#define USE_PACKAGE
#ifdef USE_PACKAGE
	#define PACKAGE_ESC		0xfa
	#define PACKAGE_START 	0xfb
	#define PACKAGE_END		0xfc
	#define PACKAGE_X1		0xff
	#define PACKAGE_X2		0xaa
	#define PACKAGE_E_ESC	0x01
	#define PACKAGE_E_START	0x02
	#define PACKAGE_E_END	0x03
	#define PACKAGE_E_X1	0x04
	#define PACKAGE_E_X2	0x05
#endif

void package_and_write(uint8_t ch){
#ifdef USE_PACKAGE
	if(ch == PACKAGE_ESC){
		osMessagePut(Q_UartWrite, PACKAGE_ESC, osWaitForever);
		osMessagePut(Q_UartWrite, PACKAGE_E_ESC, osWaitForever);
	}else if(ch == PACKAGE_START){
		osMessagePut(Q_UartWrite, PACKAGE_ESC, osWaitForever);
		osMessagePut(Q_UartWrite, PACKAGE_E_START, osWaitForever);
	}else if(ch == PACKAGE_END){
		osMessagePut(Q_UartWrite, PACKAGE_ESC, osWaitForever);
		osMessagePut(Q_UartWrite, PACKAGE_E_END, osWaitForever);
	}else if(ch == PACKAGE_X1){
		osMessagePut(Q_UartWrite, PACKAGE_ESC, osWaitForever);
		osMessagePut(Q_UartWrite, PACKAGE_E_X1, osWaitForever);
	}else if(ch == PACKAGE_X2){
		osMessagePut(Q_UartWrite, PACKAGE_ESC, osWaitForever);
		osMessagePut(Q_UartWrite, PACKAGE_E_X2, osWaitForever);
	}else{
		osMessagePut(Q_UartWrite, ch, osWaitForever);
	}
#else
	osMessagePut(Q_UartWrite, ch, osWaitForever);
#endif
}

// Called by any thread, for sending TLV frame. 
// If definded USE_PACAGE, data will be packaged.
void UART_Write_Frame(uint8_t tag, uint16_t length, void* value)
{
	uint8_t temp,i;
	const uint8_t* p = value;
	uint8_t sumcheck = 0;
	#ifdef USE_PACKAGE
		osMessagePut(Q_UartWrite, PACKAGE_START, osWaitForever);
	#endif
	// Tag
	package_and_write(tag);
	// Length
	temp = length & 0xff;
	package_and_write(temp);
	temp = length >> 8;
	package_and_write(temp);
	// Value
	for(i=0;i<length;++i){
		package_and_write(*(p+i));
		sumcheck += *(p+i);
	}
	// Sum check
	package_and_write(sumcheck);
	#ifdef USE_PACKAGE
		osMessagePut(Q_UartWrite, PACKAGE_END, osWaitForever);
	#endif
}

void Thread_uart_send (void const *argument) {
	uint8_t data;
	osEvent os_result;
	
	// Wait for uart initialize complete.
	while(flagUartInitComplete == 0)osThreadYield();

	while(1) {
		os_result = osMessageGet(Q_UartWrite, osWaitForever);
		if(os_result.status == osEventMessage){
			data = os_result.value.v;
			UrtTx(pADI_UART, data);
			osSignalWait(SIG_UART_SEND_COMPLETE, osWaitForever);
		}
	}
}

void UART_Int_Handler(void)
{
	int flag = UrtIntSta(pADI_UART);
	uint8_t data;
	volatile static uint8_t a[40];
	volatile static int point = 0;
	if((flag & COMIIR_NINT) != 0) return; 
	if((flag & COMIIR_STA_MSK) == COMIIR_STA_RXBUFFULL){ 
		// uart receive interrupt
		data = UART_read();
		a[point++] = data;
		//TODO: check if it's needed to change parameter "millisec" to 0.
		osMessagePut(Q_UartRead , data , 0);
		if(point>=40){
			point=0;
		}
	} else if((flag & COMIIR_STA_MSK) == COMIIR_STA_TXBUFEMPTY){ 
		// uart send complete interrupt
		osSignalSet(tid_Thread_uart_send,SIG_UART_SEND_COMPLETE);
	}
}
