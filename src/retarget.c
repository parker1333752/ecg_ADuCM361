#include <retarget.h>
#include <cmsis_os.h>

extern osMessageQId Q_UartRead;
extern osMessageQId Q_UartWrite;

// Override fputc, for the usage of printf.
int fputc(int ch, FILE *f)
{
    osMessagePut(Q_UartWrite, (uint8_t) ch, osWaitForever);
    return ch;
}

// Override fgetc, for the usage of scanf.
int fgetc(FILE *f)
{
	osEvent os_result = osMessageGet(Q_UartRead, osWaitForever);
	return (int)os_result.value.v;
}

int ferror(FILE *f)
{
	return EOF;
}
