#ifndef INTERRUPTS_H_
#define INTERRUPTS_H_
#include <stdint.h>

void idt_set_descriptor(uint8_t vector, void *isr, uint8_t flags);
void exception_handler(uint8_t vector, uint64_t error_code);
void irq_handler(uint8_t vector);
void idt_init(void);

#endif