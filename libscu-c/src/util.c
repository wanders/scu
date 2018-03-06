#include <assert.h>
#include <sys/syscall.h>
#include <unistd.h>

#include "util.h"

void
_scu_strlcpy(char *dst, const char *src, size_t size)
{
	size_t pos = 0;
	for (;;) {
		if (pos == size - 1) {
			dst[pos] = 0;
			break;
		}
		dst[pos] = src[pos];
		if (!src[pos])
			break;
		pos++;
	}
}

void
_scu_cescape_str(char *dest, const char *src, size_t size)
{
	assert(size >= 3);

	size_t o = 0;
	dest[o++] = '"';

	#define NEED_OUTPUT_CHARS(n) \
		if (o + (n) >= size - 2) break
	#define PUT_CHAR(c) \
		dest[o++] = c

	for (size_t i = 0; src[i]; i++) {
		unsigned char c = src[i];
		if (c == '\n') {
			NEED_OUTPUT_CHARS(2);
			PUT_CHAR('\\');
			PUT_CHAR('n');
		} else if (c == '\t') {
			NEED_OUTPUT_CHARS(2);
			PUT_CHAR('\\');
			PUT_CHAR('t');
		} else if (c < 32 || c >= 127) {
			NEED_OUTPUT_CHARS(4);
			PUT_CHAR('\\');
			PUT_CHAR('x');
			PUT_CHAR("0123456789abcdef"[c >> 4]);
			PUT_CHAR("0123456789abcdef"[c & 0xf]);
		} else {
			NEED_OUTPUT_CHARS(1);
			PUT_CHAR(c);
		}
	}
	PUT_CHAR('"');
	PUT_CHAR(0);

	#undef NEED_OUTPUT_CHARS
	#undef PUT_CHAR
}

pid_t
get_current_thread_id(void)
{
	return syscall(SYS_gettid);
}

double
get_time_diff(struct timespec startt, struct timespec endt)
{
	time_t dsec = endt.tv_sec - startt.tv_sec;
	long dnsec = endt.tv_nsec - startt.tv_nsec;

	if (dnsec < 0) {
		dnsec += 1000000000;
		dsec--;
	}

	return dsec + dnsec / 1e9;
}
