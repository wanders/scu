#ifndef _PRETTY_H_
#define _PRETTY_H_

#include <stdio.h>

static inline void __attribute__((used))
_scu_prettyprint_integer_value(char *buf, size_t size, unsigned long long value, size_t value_size)
{
	unsigned long long mask = value_size < 8 ? (1ULL << (value_size * 8)) - 1 : ~0ULL;

	if (value & (1LLU << ((value_size * 8) - 1))) {
		snprintf(buf, size, "%llu (0x%llx == %lld)", value & mask, value & mask, value | ~mask);
	} else {
		snprintf(buf, size, "%llu (0x%llx)", value & mask, value & mask);
	}
}

static inline void __attribute__((used))
_scu_prettyprint_pointer_value(char *buf, size_t size, const void *ptr)
{
	if (ptr) {
		snprintf(buf, size, "%p", ptr);
	} else {
		snprintf(buf, size, "NULL");
	}
}

static inline void __attribute__((used))
_scu_prettyprint_bytes_value(char *buf, size_t size, const void *value, size_t value_size)
{
	size_t o = 0;
	for (size_t i = 0; (o < (size - 2)) && (i < value_size); i++) {
		snprintf(buf + o, size, "%02x", ((const char *)value)[i]);
		o += 2;
	}
	buf[o] = 0;
}

#endif
