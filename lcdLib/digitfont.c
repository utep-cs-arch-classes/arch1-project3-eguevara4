#include "lcdutils.h"

const unsigned char font_digit[10][5] = {		// basic font
    { 0x3e, 0x51, 0x49, 0x45, 0x3e } // 0
  , { 0x00, 0x42, 0x7f, 0x40, 0x00 } // 1
  , { 0x42, 0x61, 0x51, 0x49, 0x46 } // 2
  , { 0x21, 0x41, 0x45, 0x4b, 0x31 } // 3
  , { 0x18, 0x14, 0x12, 0x7f, 0x10 } // 4
  , { 0x27, 0x45, 0x45, 0x45, 0x39 } // 5
  , { 0x3c, 0x4a, 0x49, 0x49, 0x30 } // 6
  , { 0x01, 0x71, 0x09, 0x05, 0x03 } // 7
  , { 0x36, 0x49, 0x49, 0x49, 0x36 } // 8
  , { 0x06, 0x49, 0x49, 0x29, 0x1e } // 9
  };