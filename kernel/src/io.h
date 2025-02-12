#ifndef IO_H_
#define IO_H_
#include <stdint.h>

inline uint8_t inb(uint16_t port);
inline void outb(uint16_t port, uint8_t val);
inline void io_wait(void);

#endif