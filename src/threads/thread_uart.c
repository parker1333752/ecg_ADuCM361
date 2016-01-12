#include "thread_uart.h"
#include "thread_adc.h"
#include "uart.h"
#include "utils.h"
#include <stdio.h>
#include "pools.h"

/* ========== Define Message Queues ========================================= */
/* Define uart write message queue.
 * For storing data that need to send from MCU. */
#define Se_UartWrite_Count 1 // only one uart peripherals.
osSemaphoreId Se_UartWrite;
osSemaphoreDef (Se_UartWrite);

#define UartWriteQueueSize 2000
MqueueDef __UartWriteQueue;
MqueueDef* const UartWriteQueue = &__UartWriteQueue;
volatile uint8_t UartTxStopped;
uint8_t __UartWriteQueueData[UartWriteQueueSize];

///* Define uart read message queue. Used for transform MPU data. */
//#define Q_UartRead_Size 30
//osMessageQId Q_UartRead;
//osMessageQDef (Q_UartRead, Q_UartRead_Size, uint32_t);

/* ========== Define thread ids ============================================= */
osThreadId tid_Thread_uart;
osThreadId tid_Thread_uart_send;

/* Definition of Uart send thread. This thread will create in Thread_uart. */
void Thread_uart_send (void const*);
osThreadDef_t ThreadDef_uart_send = {Thread_uart_send, osPriorityNormal, 1, 0};

/* ========== Define Global variables ======================================= */
/* For test if uart init complete,
 * prevent calling uart send before initialization completed. */
volatile char flagUartInitComplete = 0;

/* Thread funcion for uart data reading and processing.
 * Use message for getting data from uart read (after status machine),
 * and send this data to PC.
 */
void Thread_uart (void const *argument) {
	// Configure uart peripheral.
	UART_init();

//	// Create message queue
//	Q_UartRead = osMessageCreate(osMessageQ(Q_UartRead), NULL);
	
	// Create semaphoreCreate for uart write queue
	Se_UartWrite = osSemaphoreCreate(osSemaphore(Se_UartWrite), Se_UartWrite_Count);
	
	initQ(UartWriteQueue, UartWriteQueueSize, __UartWriteQueueData);

	// create thread uart send
	tid_Thread_uart_send = osThreadCreate(&ThreadDef_uart_send, NULL);

	// Tell other threads that initialization complete.
	flagUartInitComplete = 1;

	while(1) {
		osThreadYield();
	}
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
		pushQ(UartWriteQueue, PACKAGE_ESC);
		pushQ(UartWriteQueue, PACKAGE_E_ESC);
	}else if(ch == PACKAGE_START){
		pushQ(UartWriteQueue, PACKAGE_ESC);
		pushQ(UartWriteQueue, PACKAGE_E_START);
	}else if(ch == PACKAGE_END){
		pushQ(UartWriteQueue, PACKAGE_ESC);
		pushQ(UartWriteQueue, PACKAGE_E_END);
	}else if(ch == PACKAGE_X1){
		pushQ(UartWriteQueue, PACKAGE_ESC);
		pushQ(UartWriteQueue, PACKAGE_E_X1);
	}else if(ch == PACKAGE_X2){
		pushQ(UartWriteQueue, PACKAGE_ESC);
		pushQ(UartWriteQueue, PACKAGE_E_X2);
	}else{
		pushQ(UartWriteQueue, ch);
	}
#else
	push(UartWriteQueue, ch);
#endif
}

// Called by any thread, for sending TLV frame.
// If definded USE_PACAGE, data will be packaged.
void UART_Write_Frame(uint8_t tag, uint16_t length, void* value)
{
	uint8_t temp,i;
	uint8_t* p = value;
	uint8_t sumcheck = 0;
	
//	EcgDataDef* ptmp = value;
//	ptmp->date = sizeQ(UartWriteQueue);

	/* discards data before uart init completed. */
	if (flagUartInitComplete == 0) return;
	
	// Wait for queue available;
	osSemaphoreWait(Se_UartWrite, osWaitForever);

	#ifdef USE_PACKAGE
		pushQ(UartWriteQueue, PACKAGE_START);
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
		pushQ(UartWriteQueue, PACKAGE_END);
	#endif
	
	// release Uart Queue lock;
	osSemaphoreRelease(Se_UartWrite);
}

void Thread_uart_send (void const *argument) {
	uint8_t data;
	UartTxStopped=1;
	while(1) {
		if(emptyQ(UartWriteQueue))osThreadYield();
		else {
			if(UartTxStopped==1){
				UartTxStopped=0;
				data = popQ(UartWriteQueue);
				UrtTx(pADI_UART, data);
			}
		}
	}
}

void UART_Int_Handler(void)
{
	int flag = UrtIntSta(pADI_UART);
	volatile uint8_t data;
	if((flag & COMIIR_NINT) != 0) return;
	if((flag & COMIIR_STA_MSK) == COMIIR_STA_RXBUFFULL){
		/* uart receive interrupt */
		data = UART_read();
	} else if((flag & COMIIR_STA_MSK) == COMIIR_STA_TXBUFEMPTY){
		/* uart send complete interrupt */
		if(emptyQ(UartWriteQueue)){
			UartTxStopped=1;
		}else{
			data = popQ(UartWriteQueue);
			UrtTx(pADI_UART, data);
		}
	}
}
