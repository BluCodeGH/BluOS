#include "kb.h"
#include "stddef.h"
#include "../kernel/int/handlers.h"
#include "screen.h"

const char kbdus[128] = {
  0,  'E', '1', '2', '3', '4', '5', '6', '7', '8', /* ESC, 9 */
  '9', '0', '-', '=', '\b', /* Backspace */
  '\t',         /* Tab */
  'q', 'w', 'e', 'r',   /* 19 */
  't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', /* Enter key */
    'Q',          /* 29   - Control */
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', /* 39 */
  '\'', '`',   'S',        /* Left shift */
  '\\', 'z', 'x', 'c', 'v', 'b', 'n',            /* 49 */
  'm', ',', '.', '/',   'S',              /* Right shift */
  '*',
  'A',  /* Alt */
  ' ',  /* Space bar */
  'Z',  /* Caps lock */
  0,  /* 59 - F1 key ... > */
  0,   0,   0,   0,   0,   0,   0,   0,
  0,  /* < ... F10 */
  'X',  /* 69 - Num lock*/
  'C',  /* Scroll Lock */
  'H',  /* Home key */
  'U',  /* Up Arrow */
  '^',  /* Page Up */
  '-',
  'L',  /* Left Arrow */
   0,  
  'R',  /* Right Arrow */
  '+',
  'F',  /* 79 - End key*/
  'D',  /* Down Arrow */
  'V',  /* Page Down */
  'I',  /* Insert Key */
  '>',  /* Delete Key */
  0,   0,   0,
  0,  /* F11 Key */
  0,  /* F12 Key */
  0,  /* All other keys are undefined */
};

const char kbdusU[128] = {
  0, 0, '!', '@', '#', '$', '%', '^', '&', '*',
  '(', ')', '_', '+', 0, 0, 'Q', 'W', 'E', 'R',
  'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', 0, 0,
  'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',
  '\"', '~', 0, '|', 'Z', 'X', 'C', 'V', 'B', 'N',
  'M', '<', '>', '?', 0,
};

char mod = 0; //Bits in order LSB->MSB: Scroll Lock, Num Lock, Caps Lock, Shift, Ctrl, Alt

/* Handles the keyboard interrupt */
void kb_callback(registers_t regs) {
  unsigned char scancode, key;

  /* Read from the keyboard's data buffer */
  scancode = bin(0x60);

  key = kbdus[(int)scancode];

  /* If the top bit of the byte we read from the keyboard is
  *  set, that means that a key has just been released */
  if (scancode & 0x80) {
    scancode -= 0x80;
    key = kbdus[(int)scancode];
    switch (key) {
      case 'S': //Shift
        mod = mod & ~0x8;
        break;
      case 'Q': //Ctrl
        mod = mod & ~0x10;
        break;
      case 'A': //Alt
        mod = mod & ~0x20;
        break;
    }
  } else {
    switch (key) {
      case 'C': //Scroll Lock
        mod = mod ^ 0x1;
        break;
      case 'X': //Num Lock
        mod = mod ^ 0x2;
        break;
      case 'Z': //Caps Lock
        mod = mod ^ 0x4;
        break;
      case 'S': //Shift
        mod = mod | 0x8;
        break;
      case 'Q': //Ctrl
        mod = mod | 0x10;
        break;
      case 'A': //Alt
        mod = mod | 0x20;
        break;
      case 'U':
        move_cursor(-1, 0);
        break;
      case 'D':
        move_cursor(1, 0);
        break;
      case 'L':
        move_cursor(0, -1);
        break;
      case 'R':
        move_cursor(0, 1);
        break;
      default:
        if ((mod & 0x30) == 0) { //No CTRL or Alt
          if (((mod & 0xB) == 0) || kbdusU[(int)scancode] == 0) { //No Shift and no Caps-Lock or Non-Shiftable
            if (key != 0 && (key < 65 || key > 90)) { //Its not a control character (capital)
              print_char(key);
            }
          }
          else {
            print_char(kbdusU[(int)scancode]);
          }
        }
    }
  }
}

void setup_kb() {
  register_interrupt_handler(IRQ1, &kb_callback);
}