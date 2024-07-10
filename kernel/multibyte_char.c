#include "types.h"
#include "multibyte_char.h"

uint64 m_strlen(MultiChar *mc) {
    uint64 result = 0;
    while (1) {
        if (mc->len == 1 && mc->data[0] == '\0') {
            break;
        }
        result++;
        mc++;
    }
    return result;
}

void m_safestrcpy(MultiChar *dest, const MultiChar *from, int length) {
    for (int i = 0; i < length; i++) {
        dest[i].len = from[i].len;
        for (int j = 0; j < from->len; j++) {
            dest[i].data[j] = from[i].data[j];
        }
    }
}
