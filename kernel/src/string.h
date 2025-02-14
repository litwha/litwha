#ifndef STRING_H_
#define STRING_H_
#include <stdint.h>
#include <stddef.h>
size_t strlen(const char *str);
void iota(char *buffer, uint8_t num);
void iota64(char *buffer, unsigned int num);
void iotx(char *buffer, uint8_t num, uint8_t pad);
#endif