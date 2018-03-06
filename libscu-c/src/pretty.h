#ifndef _PRETTY_H_
#define _PRETTY_H_

#include <stdio.h>

static inline void __attribute__((used))
_scu_prettyprint_numeric_value(char *buf, size_t bufsiz, unsigned long long value, size_t bitlen)
{
	unsigned long long mask;
	if (bitlen == 64) {
		mask = 0xffffffffffffffff;
	} else {
		mask = (1ULL << (bitlen * 8)) - 1;
	}
	if (value & 1LLU << ((bitlen * 8) - 1)) {
		snprintf(buf, bufsiz, "%llu (0x%llx == %lld)", value & mask, value & mask, value | ~mask);
	} else {
		snprintf(buf, bufsiz, "%llu (0x%llx)", value & mask, value & mask);
	}
}

static inline void __attribute__((used))
_scu_prettyprint_pointer_value(char *buf, size_t bufsiz, const void *value)
{
	if (value) {
		snprintf(buf, bufsiz, "%p", value);
	} else {
		snprintf(buf, bufsiz, "NULL");
	}
}

#endif
