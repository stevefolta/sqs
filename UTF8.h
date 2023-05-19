#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

struct String;


extern bool is_valid_utf8(const char* bytes, int num_bytes);
extern int bytes_in_utf8_character(uint8_t byte);
extern int put_utf8(uint32_t character, char* utf8_out);
extern struct String* decode_8859_1(const uint8_t* bytes, size_t size);

