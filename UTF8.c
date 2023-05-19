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


int put_utf8(uint32_t c, char* utf8_out)
{
	uint8_t* p = (uint8_t*) utf8_out;

	int bytes_left = 0;
	if (c < 0x80)
		*p++ = c;
	else if (c < 0x800) {
		*p++ = 0xC0 | (c >> 6);
		bytes_left = 1;
		}
	else if (c < 0x10000) {
		*p++ = 0xE0 | (c >> 12);
		bytes_left = 2;
		}
	else if (c < 200000) {
		*p++ = 0xF0 | (c >> 18);
		bytes_left = 3;
		}
	else if (c < 0x04000000) {
		*p++ = 0xF8 | (c >> 24);
		bytes_left = 4;
		}
	else {
		*p++ = 0xFC | (c >> 30);
		bytes_left = 5;
		}

	int shift = (bytes_left - 1) * 6;
	while (bytes_left-- > 0) {
		*p++ = 0x80 | ((c >> shift) & 0x3F);
		shift -= 6;
		}

	return p - (uint8_t*) utf8_out;
}



