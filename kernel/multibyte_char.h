#include "types.h"

struct multichar {
    char data[8];
    ushort len;
} typedef MultiChar;

uint64 m_strlen(MultiChar *mc);
void m_safestrcpy(MultiChar *dest, const MultiChar *from, int length);