#ifndef SERIAL_H_
#define SERIAL_H_
#include <stdint.h>

int init_serial();
char read_serial();
void write_serial_str(char* str);

#endif