#include <stdint.h>
#include <stdbool.h>
#include "interrupts.h"
#include "../serial/serial.h"
#include "../pic/pic.h"
#include "../string.h"

typedef struct
{
    uint16_t isr_low;   // The lower 16 bits of the ISR's address
    uint16_t kernel_cs; // The GDT segment selector that the CPU will load into CS before calling the ISR
    uint8_t ist;        // The IST in the TSS that the CPU will load into RSP; set to zero for now
    uint8_t attributes; // Type and attributes; see the IDT page
    uint16_t isr_mid;   // The higher 16 bits of the lower 32 bits of the ISR's address
    uint32_t isr_high;  // The higher 32 bits of the ISR's address
    uint32_t reserved;  // Set to zero
} __attribute__((packed)) idt_entry_t;

typedef struct
{
    uint16_t limit;
    uint64_t base;
} __attribute__((packed)) idtr_t;

__attribute__((aligned(0x10))) static idt_entry_t idt[256];

static idtr_t idtr;

void exception_handler(uint8_t vector, uint64_t error_code)
{
    char vec_str[4], err_hex[10];
    iota(vec_str, vector);

    iota(err_hex, error_code);

    write_serial_str("\nEXCEPTION ");
    write_serial_str(vec_str);
    write_serial_str(" | Error: ");
    write_serial_str(err_hex);
    // ... rest of handler

    // Print CR2 for page faults
    if (vector == 14)
    {
        uint64_t cr2;
        asm volatile("mov %%cr2, %0" : "=r"(cr2));
        char cr2_str[12];
        iota(cr2_str, cr2);
        write_serial_str("\nCR2: ");
        write_serial_str(cr2_str);
        write_serial_str("\n");
    }

    __asm__ volatile("cli\n hlt");
}

unsigned int tick = 0;
void irq_handler(uint8_t vector)
{
    uint8_t corrected = vector - 32;

    PIC_sendEOI(corrected); // Convert to IRQ number

    // Add your IRQ handling logic here
    if (corrected == 0)
    { // Timer interrupt (IRQ0)
        /*
        tick++;
        char tick_str[16];
        iota64(tick_str, tick);
        write_serial_str("tick ");
        write_serial_str(tick_str);
        write_serial_str("    \r");
        */
        ;
    }
    else if (corrected == 1)
    { // Keyboard (IRQ1) - We shouldn't read the keycode here! That could result in us sending invalid codes due to a badly timed interrupt!
      // So we just set that we have key[s] to check.
      // TODO: this
    }
    else
    {
        char str[12];
        iota(str, corrected);
        write_serial_str("IRQ ");
        write_serial_str(str);
        write_serial_str(": ");
        write_serial_str("\n");
    }
}

void idt_set_descriptor(uint8_t vector, void *isr, uint8_t flags)
{
    idt_entry_t *descriptor = &idt[vector];

    descriptor->isr_low = (uint64_t)isr & 0xFFFF;
    descriptor->kernel_cs = 0x28;
    descriptor->ist = 0;
    descriptor->attributes = flags;
    descriptor->isr_mid = ((uint64_t)isr >> 16) & 0xFFFF;
    descriptor->isr_high = ((uint64_t)isr >> 32) & 0xFFFFFFFF;
    descriptor->reserved = 0;
}

static bool vectors[256];

extern void *isr_stub_table[];

void idt_init()
{
    idtr.base = (uintptr_t)&idt[0];
    idtr.limit = (uint16_t)sizeof(idt_entry_t) * 256 - 1;

    for (uint8_t vector = 0; vector < 32; vector++)
    {
        idt_set_descriptor(vector, isr_stub_table[vector], 0x8E);
        vectors[vector] = true;
    }

    for (uint8_t vector = 32; vector < 47; vector++)
    {
        idt_set_descriptor(vector, isr_stub_table[vector], 0x8E);
        vectors[vector] = true;
    }

    __asm__ volatile("lidt %0" : : "m"(idtr)); // load the new IDT
    __asm__ volatile("sti");                   // set the interrupt flag
}