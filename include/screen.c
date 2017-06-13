#include "screen.h"
#include "stddef.h"
#include "ports.h"
#include "mem.h"

int old_offset = 0;

/* Print a char on the screen at row , col , or at cursor position */
void _print_char(char character, int row, int col, char attribute_byte) {
  /* Create a byte ( char ) pointer to the start of video memory */
  unsigned char *vidmem = (unsigned char *) VIDEO_ADDRESS;
  /* If attribute byte is zero , assume the default style . */
  if (!attribute_byte) {
    attribute_byte = WHITE_ON_BLACK ;
  }
  /* Get the video memory offset for the screen location */
  int offset;
  /* If col and row are non - negative , use them for offset . */
  if (col >= 0 && row >= 0) {
    offset = get_screen_offset(row, col);
    /* Otherwise , use the current cursor position . */
  } else {
    offset = get_cursor();
  }
  // If we see a newline character , set offset to the end of
  // current row , so it will be advanced to the first col
  // of the next row.
  if (character == '\n') {
    int row = offset / (2 * MAX_COLS);
    offset = get_screen_offset(row, 79);
  } else if (character == '\b') {
    offset = old_offset;
    vidmem[offset] = ' ';
    vidmem[offset+1] = attribute_byte;
    if (offset >= 0) {
      offset -= 2; //counteract += 2 to move cursor back.
    }
  } else if (character == '\t') {
    int i;
    for (i = 0; i < TAB_WIDTH; i++) {
      vidmem[offset] = ' ';
      vidmem[offset+1] = attribute_byte;
      offset += 2;
    }
    offset -= 2;
  } else {
    vidmem[offset] = character;
    vidmem[offset+1] = attribute_byte;
  }
  // Update the offset to the next character cell , which is
  // two bytes ahead of the current cell .
  old_offset = offset;
  offset += 2;
  // Make scrolling adjustment , for when we reach the bottom
  // of the screen .
  offset = handle_scrolling(offset);
  // Update the cursor position on the screen device .
  set_cursor(offset);
}

int get_screen_offset(int row, int col) {
  return (row * 80 + col) * 2;
}

int get_cursor() {
  // The device uses its control register as an index
  // to select its internal registers , of which we are
  // interested in:
  // reg 14: which is the high byte of the cursor ’s offset
  // reg 15: which is the low byte of the cursor ’s offset
  // Once the internal register has been selected , we may read or
  // write a byte on the data register .
  bout(REG_SCREEN_CTRL, 14);
  int offset = bin(REG_SCREEN_DATA) << 8;
  bout(REG_SCREEN_CTRL, 15);
  offset += bin(REG_SCREEN_DATA);
  // Since the cursor offset reported by the VGA hardware is the
  // number of characters , we multiply by two to convert it to
  // a character cell offset.
  return offset *2;
}

void set_cursor(int offset) {
  offset = offset / 2;
  // This is similar to get_cursor , only now we write
  // bytes to those internal device registers .
  bout(REG_SCREEN_CTRL, 14);
  bout(REG_SCREEN_DATA, (unsigned char)(offset >> 8));
  bout(REG_SCREEN_CTRL, 15);
  bout(REG_SCREEN_DATA, (unsigned char)(offset & 0xFF));
}

void reset_cursor() {
  wout(0x3D4, 0xE0A);
  wout(0x3D4, 0xF0B);
}

void move_cursor(int drow, int dcol) {
  int offset;
  offset = get_cursor();
  offset += get_screen_offset(drow, dcol); //works for negatives too.
  if (offset >= 0 && offset < MAX_ROWS * MAX_COLS * 2) {
    set_cursor(offset);
  }
}

void print_char(char c) {
  _print_char(c, -1, -1, WHITE_ON_BLACK);
}

void print_at(char* message, int col, int row) {
  // Update the cursor if col and row not negative .
  if (col >= 0 && row >= 0) {
    set_cursor(get_screen_offset(col, row));
  }
  // Loop through each char of the message and print it.
  int i = 0;
  while (message[i] != 0) {
    _print_char(message[i++], -1, -1, WHITE_ON_BLACK);
  }
}

void print(char* message) {
  print_at(message, -1, -1);
}

void clear_screen() {
  int row = 0;
  int col = 0;
  /* Loop through video memory and write blank characters . */
  for (row = 0; row < MAX_ROWS; row++) {
    for (col = 0; col < MAX_COLS; col++) {
      _print_char(' ', row, col, WHITE_ON_BLACK);
    }
  }
  // Move the cursor back to the top left .
  set_cursor(get_screen_offset(0, 0));
}

/* Advance the text cursor , scrolling the video buffer if necessary . */
int handle_scrolling(int cursor_offset) {
  // If the cursor is within the screen , return it unmodified .
  if (cursor_offset < MAX_ROWS * MAX_COLS * 2) {
    return cursor_offset;
  }
  /* Shuffle the rows back one . */
  int i;
  for (i = 1; i < MAX_ROWS; i++) {
    memcpy(VIDEO_ADDRESS + get_screen_offset(i, 0),
      VIDEO_ADDRESS + get_screen_offset(i - 1, 0),
      MAX_COLS * 2
    );
  }
  /* Blank the last line by setting all bytes to 0 */
  char* last_line = get_screen_offset(MAX_ROWS - 1, 0) + VIDEO_ADDRESS;
  for (i = 0; i < MAX_COLS * 2; i++) {
    last_line[i] = 0;
  }
  // Move the offset back one row , such that it is now on the last
  // row , rather than off the edge of the screen .
  cursor_offset -= 2 * MAX_COLS;
  // Return the updated cursor position .
  return cursor_offset ;
}

void print_hex(u32int n) {
  s32int tmp;
  print("0x");
  int i;
  for (i = 28; i > 0; i -= 4)
  {
    tmp = (n >> i) & 0xF;
    if (tmp >= 0xA) {
      _print_char(tmp-0xA+'a', -1, -1, WHITE_ON_BLACK);
    } else {
      _print_char( tmp+'0', -1, -1, WHITE_ON_BLACK);
    }
  }
  tmp = n & 0xF;
  if (tmp >= 0xA) {
    _print_char(tmp-0xA+'a', -1, -1, WHITE_ON_BLACK);
  } else {
    _print_char( tmp+'0', -1, -1, WHITE_ON_BLACK);
  }

}

void print_dec(u32int n) {
  if (n == 0) {
    _print_char('0', -1, -1, WHITE_ON_BLACK);
    return;
  }
  s32int acc = n;
  char c[32];
  int i = 0;
  while (acc > 0) {
    c[i] = '0' + acc%10;
    acc /= 10;
    i++;
  }
  c[i] = 0;
  char c2[32];
  c2[i--] = 0;
  int j = 0;
  while(i >= 0) {
    c2[i--] = c[j++];
  }
  print(c2);
}