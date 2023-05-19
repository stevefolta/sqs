#pragma once

#include <stdint.h>
#include <stdbool.h>


extern bool is_valid_utf8(const char* bytes, int num_bytes);
extern int bytes_in_utf8_character(uint8_t byte);
extern int put_utf8(uint32_t character, char* utf8_out);

