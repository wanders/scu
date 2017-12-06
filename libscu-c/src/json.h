#ifndef _JSON_H_
#define _JSON_H_

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define _SCU_JSON_STRING_LENGTH 1024

#define _SCU_ASCII_BACKSLASH '\\'
#define _SCU_ASCII_DOUBLE_QUOTE '"'

static inline int __attribute__((used))
json_true(int fd)
{
	return write(fd, "true", 4);
}

static inline int __attribute__((used))
json_false(int fd)
{
	return write(fd, "false", 5);
}

#define json_boolean(stream, val) ((val) ? json_true(stream) : json_false(stream))

static inline void __attribute__((used))
json_escape_string(char *out, const char *in)
{
	size_t out_i = 0;
	for (size_t in_i = 0; (in_i < _SCU_JSON_STRING_LENGTH) && (out_i < _SCU_JSON_STRING_LENGTH - 1) && (in[in_i] != 0); in_i++) {
		if (in[in_i] == _SCU_ASCII_BACKSLASH || in[in_i] == _SCU_ASCII_DOUBLE_QUOTE) {
			if (out_i >= _SCU_JSON_STRING_LENGTH - 1)
				break;
			out[out_i++] = _SCU_ASCII_BACKSLASH;
		} else if (in[in_i] == '\n') {
			if (out_i >= _SCU_JSON_STRING_LENGTH - 2)
				break;
			out[out_i++] = _SCU_ASCII_BACKSLASH;
			out[out_i++] = 'n';
			continue;
		}
		out[out_i++] = in[in_i];
	}
	out[out_i] = 0;
}

static inline int __attribute__((used))
json_string(int fd, const char *value)
{
	char escaped[_SCU_JSON_STRING_LENGTH];
	json_escape_string(escaped, value);
	return dprintf(fd, "\"%s\"", escaped);
}

static inline int __attribute__((used))
json_integer(int fd, int value)
{
	return dprintf(fd, "%d", value);
}

static inline int __attribute__((used))
json_real(int fd, double value)
{
	return dprintf(fd, "%f", value);
}

static inline int __attribute__((used))
json_object_start(int fd)
{
	return write(fd, "{", 1);
}

static inline int __attribute__((used))
json_object_key(int fd, const char *key)
{
	return dprintf(fd, "\"%s\": ", key);
}

static inline int __attribute__((used))
json_object_end(int fd)
{
	return write(fd, "}", 1);
}

static inline int __attribute__((used))
json_array_start(int fd)
{
	return write(fd, "[", 1);
}

static inline int __attribute__((used))
json_array_end(int fd)
{
	return write(fd, "]", 1);
}

static inline int __attribute__((used))
json_separator(int fd)
{
	return write(fd, ", ", 2);
}

#endif
