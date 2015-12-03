#ifndef __RETARGET_H__
#define __RETARGET_H__

#include <stdio.h>
#define frame_end 0xfa
#define escape_char 0xfb
#define escape_0xfa 0xfb
#define escape_0xfb 0xfc

int fputc(int ch, FILE *f);
void put_frame_end(void);

#endif
