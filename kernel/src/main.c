#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>
#include "./io/io.h"
#include "./pic/pic.h"
#include "./serial/serial.h"
#include "./interrupts/interrupts.h"
#define PIC1 0x20 /* IO base address for master PIC */
#define PIC2 0xA0 /* IO base address for slave PIC */
#define PIC1_COMMAND PIC1
#define PIC1_DATA (PIC1 + 1)
#define PIC2_COMMAND PIC2
#define PIC2_DATA (PIC2 + 1)

// Set the base revision to 3, this is recommended as this is the latest
// base revision described by the Limine boot protocol specification.
// See the Limine specification for further info.

__attribute__((used, section(".limine_requests"))) static volatile LIMINE_BASE_REVISION(3);

// The Limine requests can be placed anywhere, but it is important that
// the compiler does not optimise them away, so, usually, they should
// be made volatile or equivalent, _and_ they should be accessed at least
// once or marked as used with the "used" attribute as done here.

__attribute__((used, section(".limine_requests"))) static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0};

// Finally, define the start and end markers for the Limine requests.
// These can also be moved anywhere, to any .c file, as seen fit.

__attribute__((used, section(".limine_requests_start"))) static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests_end"))) static volatile LIMINE_REQUESTS_END_MARKER;

// GCC and Clang reserve the right to generate calls to the following
// 4 functions even if they are not directly called.
// Implement them as the C specification mandates.
// DO NOT remove or rename these functions, or stuff will eventually break!
// They CAN be moved to a different .c file.

void *memcpy(void *dest, const void *src, size_t n)
{
    uint8_t *pdest = (uint8_t *)dest;
    const uint8_t *psrc = (const uint8_t *)src;

    for (size_t i = 0; i < n; i++)
    {
        pdest[i] = psrc[i];
    }

    return dest;
}

void *memset(void *s, int c, size_t n)
{
    uint8_t *p = (uint8_t *)s;

    for (size_t i = 0; i < n; i++)
    {
        p[i] = (uint8_t)c;
    }

    return s;
}

void *memmove(void *dest, const void *src, size_t n)
{
    uint8_t *pdest = (uint8_t *)dest;
    const uint8_t *psrc = (const uint8_t *)src;

    if (src > dest)
    {
        for (size_t i = 0; i < n; i++)
        {
            pdest[i] = psrc[i];
        }
    }
    else if (src < dest)
    {
        for (size_t i = n; i > 0; i--)
        {
            pdest[i - 1] = psrc[i - 1];
        }
    }

    return dest;
}

int memcmp(const void *s1, const void *s2, size_t n)
{
    const uint8_t *p1 = (const uint8_t *)s1;
    const uint8_t *p2 = (const uint8_t *)s2;

    for (size_t i = 0; i < n; i++)
    {
        if (p1[i] != p2[i])
        {
            return p1[i] < p2[i] ? -1 : 1;
        }
    }

    return 0;
}

// Halt and catch fire function.
static void stop(void)
{
    write_serial_str("Aborting.\n");
    asm("cli");
    for (;;)
    {
        asm("hlt");
    }
}

static void spin(void)
{
    write_serial_str("Spinning.\n");
    for (;;)
    {
        asm("hlt");
    }
}

static inline bool are_interrupts_enabled()
{
    unsigned long flags;
    asm volatile("pushf\n\t"
                 "pop %0"
                 : "=g"(flags));
    return flags & (1 << 9);
}

void kernel_entrypoint(void)
{
    int SERIAL_STATUS = init_serial();
    write_serial_str("Kernel entered.\n");

    write_serial_str("Initializing interrupt descriptor table.\n");
    idt_init();
    write_serial_str("IDT initialized.\n");

    write_serial_str("Remapping PIC.\n");
    pic_remap(0x20, 0x28); // Master PIC: 0x20, Slave PIC: 0x28
    write_serial_str("PIC remapped.\n");

    if (are_interrupts_enabled())
    {
        write_serial_str("Interrupts are enabled!\n");
    }
    else
    {
        write_serial_str("Interrupts are not enabled!\n");
    }

    // // After PIC remap
    // outb(PIC1_DATA, 0xFE); // Unmask IRQ0 (timer)
    // outb(PIC2_DATA, 0xFF); // Mask all slave IRQs

    // Program PIT (timer)
    outb(0x43, 0x36);       // Channel 0, rate generator
    outb(0x40, 100 & 0xFF); // Low byte of divisor (100Hz)
    outb(0x40, 100 >> 8);   // High byte of divisor

    if (LIMINE_BASE_REVISION_SUPPORTED == false)
    {
        stop();
    }

    // Ensure we got a framebuffer.
    if (framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1)
    {
        stop();
    }

    // Fetch the first framebuffer.
    struct limine_framebuffer *framebuf = framebuffer_request.response->framebuffers[0];

    // Note: we assume the framebuffer model is RGB with 32-bit pixels for right now.
    for (size_t i = 0; i < 255; i++)
    {
        for (size_t j = 0; j < 255; j++)
        {
            volatile uint32_t *fb_ptr = framebuf->address;
            uint32_t color = (i << framebuf->red_mask_shift) + (j << framebuf->green_mask_shift);
            fb_ptr[i * (framebuf->pitch / (framebuf->bpp >> 3)) + j] = color;
        }
    }

    // We're done with initialization, so we should start spinning until an interrupt.
    spin();
}
