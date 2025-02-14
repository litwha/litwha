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
	if (num == 0)
	{
		buffer[0] = '0';
		buffer[1] = '\0';
		return;
	}

	int len = 0;
	uint8_t n = num;
	while (n > 0)
	{
		len++;
		n /= 10;
	}

	for (int i = len - 1; i >= 0; i--)
	{
		buffer[i] = (num % 10) + '0';
		num /= 10;
	}
	buffer[len] = '\0';
}

void iota64(char *buffer, unsigned int num)
{
	if (num == 0)
	{
		buffer[0] = '0';
		buffer[1] = '\0';
		return;
	}

	int len = 0;
	unsigned int n = num;
	while (n > 0)
	{
		len++;
		n /= 10;
	}

	for (int i = len - 1; i >= 0; i--)
	{
		buffer[i] = (num % 10) + '0';
		num /= 10;
	}
	buffer[len] = '\0';
}

// this doesnt work...
void iotx(char *buffer, uint8_t num, uint8_t pad)
{
	const char hex[] = "0123456789ABCDEF";

	for (int i = pad; i >= 0; i--)
	{
		buffer[i] = hex[(num >> (60 - i * 4)) & 0xF];
	}

	buffer[pad] = '\0';
}