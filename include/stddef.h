#pragma once

typedef unsigned int   u32int;
typedef          int   s32int;
typedef unsigned short u16int;
typedef          short s16int;
typedef unsigned char  u8int;
typedef          char  s8int;

typedef struct { 
    char data[256];
    unsigned int len;
} buffer256;