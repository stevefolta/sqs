#pragma once

extern void Error(const char* message, ...);

// For error messages, builds a C string.
struct String;
extern const char* where(int line_number, struct String* filename);


