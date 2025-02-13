#ifndef PIC_H_
#define PIC_H_
#include "../io/io.h"

void pic_remap();
void pic_disable();
void PIC_sendEOI(uint8_t irq);

#endif