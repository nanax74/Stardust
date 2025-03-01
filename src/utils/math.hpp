#pragma once

#define CMP_EQ 0x00 // Equal
#define CMP_NE 0x01 // Not Equal
#define CMP_GT 0x02 // Greater than
#define CMP_GE 0x03 // Greater than or equal
#define CMP_LE 0x04 // Less than or equal
#define CMP_LT 0x05 // Less than

bool Compare(int a, int b, char op);