#pragma once
#include "stddef.h"

u8int bin (u16int port);

void bout (u16int port, u8int data);

u16int win ( u16int port );

void wout ( u16int port , u16int data );