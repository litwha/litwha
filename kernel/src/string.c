#include <stddef.h>
#include "string.h"
#include <stdint.h>

size_t strlen(const char *str)
{
	size_t len = 0;
	while (str[len])
		len++;
	return len;
}

void iota(char *buffer, uint8_t num)
{
	int i, rem, len = 0, n;

	n = num;
	while (n != 0)
	{
		len++;
		n /= 10;
	}
	for (i = 0; i < len; i++)
	{
		rem = num % 10;
		num = num / 10;
		buffer[len - (i + 1)] = rem + '0';
	}
	buffer[len] = '\0';
}