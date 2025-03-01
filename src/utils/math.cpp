#include "math.hpp"

bool Compare(int a, int b, char op)
{
    switch (op) 
    {
        case CMP_EQ: return a == b;
        case CMP_NE: return a != b;
        case CMP_GT: return a > b;
        case CMP_GE: return a >= b;
        case CMP_LE: return a <= b;
        case CMP_LT: return a < b;
        default: return false;
    }
}