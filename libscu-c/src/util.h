#ifndef _UTIL_H_
#define _UTIL_H_

#include <stddef.h>
#include <time.h>
#include <unistd.h>

void _scu_strlcpy(char *dst, const char *src, size_t size);
void _scu_cescape_str(char *dest, const char *src, size_t size);
pid_t get_current_thread_id(void);
double get_time_diff(struct timespec startt, struct timespec endt);

#endif
