#ifndef __UTILS_H__
#define __UTILS_H__

#include <stdint.h>

extern float const sin200[200];
extern float const cos200[200];

#define MAX_PASSWORD_SIZE 20
typedef struct{
    char v[MAX_PASSWORD_SIZE];
    int16_t next[MAX_PASSWORD_SIZE];
	int32_t length;
}PasswordDef;

typedef union{
    char bytes[2];
    int16_t value;
}int16_bytes;

typedef union{
    char bytes[4];
    int32_t value;
}int32_bytes;

typedef union{
    char bytes[4];
    float value;
}float_bytes;

typedef union{
    char bytes[8];
    double value;
}double_bytes;

void InitPassword(PasswordDef*, char*);
char checkPassword(PasswordDef*, char);

#endif
