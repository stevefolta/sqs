#include "UTF8.h"
#include "String.h"
#include "Memory.h"


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


int bytes_in_n_characters(const char* bytes, int num_characters)
{
	int total_bytes = 0;
	for (; num_characters > 0; --num_characters) {
		int char_bytes = bytes_in_utf8_character(*bytes);
		if (char_bytes < 0)
			return -1;
		total_bytes += char_bytes;
		bytes += char_bytes;
		}
	return total_bytes;
}


int chars_in_utf8(const char* bytes_in, int num_bytes)
{
	int total_chars = 0;
	const uint8_t* bytes = (const uint8_t*) bytes_in;
	const uint8_t* end = bytes + num_bytes;
	while (bytes < end) {
		int char_bytes = bytes_in_utf8_character(*bytes);
		if (char_bytes < 0)
			return -1;
		total_chars += 1;
		bytes += char_bytes;
		if (bytes > end)
			return -1;
		}
	return total_chars;
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


String* decode_8859_1(const uint8_t* bytes, size_t size)
{
	// First, figure out how many bytes we'll need.
	size_t extra_bytes = 0;
	const uint8_t* p = bytes;
	const uint8_t* end = bytes + size;
	while (p < end) {
		uint8_t c = *p++;
		if (c >= 0x80) {
			if (c < 0xA0) {
				// These characters are actually Windows-1252, and are not valid in
				// ISO-8859-1, but standard practice is to accept Windows-1252 (a
				// superset of ISO-8859-1) whenever ISO-8859-1 is specified.
				// These characters range from U+0152 to U+2122, which all are
				// represented in 3 bytes in UTF-8.
				extra_bytes += 2;
				}
			else
				extra_bytes += 1;
			}
		}

	static uint32_t cp_1252_chars[32] = {
		0x20AC, 0, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021,
		0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0, 0x017D, 0,
		0, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
		0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0, 0x017E, 0x0178,
		};

	// Decode.
	char* utf8_bytes = alloc_mem(size + extra_bytes);
	p = bytes;
	char* out = utf8_bytes;
	while (p < end) {
		uint8_t c = *p++;
		if (c < 0x80)
			*out++ = (char) c;
		else if (c < 0xA0)
			out += put_utf8(cp_1252_chars[c - 0x80], out);
		else
			out += put_utf8(c, out);
		}

	// Make and return the String.
	String* result = alloc_obj(String);
	result->class_ = &String_class;
	result->str = utf8_bytes;
	result->size = size + extra_bytes;
	return result;
}



