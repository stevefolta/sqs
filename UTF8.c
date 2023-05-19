#include "UTF8.h"


int bytes_in_utf8_character(uint8_t byte)
{
	if ((byte & 0x80) == 0)
		return 1;

	if ((byte & 0xC0) == 0x80 || byte == 0xC0 || byte == 0xC1)
		return -1;
	int num_bytes = 1;
	while (true) {
		num_bytes += 1;
		byte <<= 1;
		if ((byte & 0xC0) == 0x80)
			break;
		if (byte == 0)
			return -1;
		}
	return (num_bytes > 6 ? -1 : num_bytes);
}


bool is_valid_utf8(const char* bytes, int num_bytes)
{
	const uint8_t* p = (const uint8_t*) bytes;
	const uint8_t* end = p + num_bytes;
	while (p < end) {
		// Single-byte characters.
		uint8_t c = *p++;
		if ((c & 0x80) == 0)
			continue;

		// How many bytes?
		int bytes_left = bytes_in_utf8_character(c) - 1;
		if (bytes_left < 0 || p + bytes_left > end)
			return false;

		while (bytes_left-- > 0) {
			if ((*p++ & 0xC0) != 0x80)
				return false;
			}
		}

	return true;
}


