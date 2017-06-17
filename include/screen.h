#pragma once
#include "stddef.h"

#define VIDEO_ADDRESS ((char *)0xb8000)
#define MAX_ROWS 25
#define MAX_COLS 80
// Attribute byte for our default colour scheme .
#define WHITE_ON_BLACK 0x0f
// Screen device I/O ports
#define REG_SCREEN_CTRL 0x3D4
#define REG_SCREEN_DATA 0x3D5

#define TAB_WIDTH 2

void _print_char(char character, int col, int row, char attribute_byte);

int get_screen_offset(int row, int col);

int get_cursor();

void set_cursor(int offset);

void reset_cursor();

void move_cursor(int drow, int dcol);

u32int print_at (char* message, int col, int row);

u32int print(char* message);

void print_char(char c);

void clear_screen();

int handle_scrolling(int cursor_offset);

void print_hex(u32int n);

u32int print_dec(u32int n);