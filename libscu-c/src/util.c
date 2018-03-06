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
	int o = 0;

	assert(size >= 3);

	dest[o++] = '"';

	for (int i = 0; src[i]; i++) {
		unsigned char c = src[i];
		if (c == '\n') {
			if (o + 2 >= size - 2)
				break;
			dest[o++] = '\\';
			dest[o++] = 'n';
		} else if (c == '\t') {
			if (o + 2 >= size - 2)
				break;
			dest[o++] = '\\';
			dest[o++] = 't';
		} else if (c < 32 || c >= 127) {
			if (o + 4 >= size - 2)
				break;
			dest[o++] = '\\';
			dest[o++] = 'x';
			dest[o++] = "0123456789abcdef"[c >> 4];
			dest[o++] = "0123456789abcdef"[c & 0xf];
		} else {
			if (o + 1 >= size - 2)
				break;
			dest[o++] = c;
		}
	}
	dest[o++] = '"';
	dest[o++] = 0;
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
