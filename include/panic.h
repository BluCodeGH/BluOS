#pragma once
#include "stddef.h"
#include "screen.h"

#define PANIC(msg) panic(msg, __FILE__, __LINE__);
#define ASSERT(b) ((b) ? (void)0 : panic_assert(__FILE__, __LINE__, #b))

void panic(char *message, char *file, u32int line);
void panic_assert(char *file, u32int line, char *desc);