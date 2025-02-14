#include "pic.h"
#include "../io/io.h"
#include "../serial/serial.h"

#define PIC1 0x20 /* IO base address for master PIC */
#define PIC2 0xA0 /* IO base address for slave PIC */
#define PIC1_COMMAND PIC1
#define PIC1_DATA (PIC1 + 1)
#define PIC2_COMMAND PIC2
#define PIC2_DATA (PIC2 + 1)

#define ICW1_INIT 0x10 // Initialization command
#define ICW1_ICW4 0x01 // Indicates ICW4 is needed
#define ICW4_8086 0x01 // 8086 mode

void pic_remap(int master_offset, int slave_offset)
{
    outb(PIC1, ICW1_INIT | ICW1_ICW4);
    io_wait();
    outb(PIC2, ICW1_INIT | ICW1_ICW4);
    io_wait();

    outb(PIC1_DATA, master_offset);
    io_wait();
    outb(PIC2_DATA, slave_offset);
    io_wait();

    outb(PIC1_DATA, 4); // tell master pic that there is a slave pic at irq2
    io_wait();
    outb(PIC2_DATA, 2); // tell slave pic it's cascade identity (where interrupts cascade down from?)
    io_wait();

    outb(PIC1_DATA, ICW4_8086); // ICW4: have the PICs use 8086 mode (and not 8080 mode)
    io_wait();
    outb(PIC2_DATA, ICW4_8086);
    io_wait();

    // Unmask both PICs.
    outb(PIC1_DATA, 0);
    outb(PIC2_DATA, 0);
}

void pic_disable()
{
    outb(PIC1_DATA, 0xff);
    outb(PIC2_DATA, 0xff);
}

#define PIC_EOI 0x20

void PIC_sendEOI(uint8_t irq)
{
    if (irq >= 8)
        outb(PIC2_COMMAND, PIC_EOI);

    outb(PIC1_COMMAND, PIC_EOI);
}
