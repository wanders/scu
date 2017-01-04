#ifndef _JSON_H_
#define _JSON_H_

#include <stdio.h>
#include <string.h>
#include <unistd.h>

static inline int __attribute__ ((used))
json_true (int fd)
{
  return write (fd, "true", 4);
}

static inline int __attribute__ ((used))
json_false (int fd)
{
  return write (fd, "false", 5);
}

#define json_boolean(stream, val) ((val) ? json_true (stream) : json_false(stream))

static inline int __attribute__ ((used))
json_string (int fd, const char *value)
{
  return dprintf (fd, "\"%s\"", value);
}

static inline int __attribute__ ((used))
json_integer (int fd, int value)
{
  return dprintf (fd, "%d", value);
}

static inline int __attribute__ ((used))
json_real(int fd, double value)
{
  return dprintf (fd, "%f", value);
}

static inline int __attribute__ ((used))
json_object_start (int fd)
{
  return write (fd, "{", 1);
}

static inline int __attribute__ ((used))
json_object_key (int fd, const char *key)
{
  return dprintf (fd, "\"%s\": ", key);
}

static inline int __attribute__ ((used))
json_object_end (int fd)
{
  return write (fd, "}", 1);
}

static inline int __attribute__ ((used))
json_array_start (int fd)
{
  return write (fd, "[", 1);
}

static inline int __attribute__ ((used))
json_array_end (int fd)
{
  return write (fd, "]", 1);
}

static inline int __attribute__ ((used))
json_separator (int fd)
{
  return write (fd, ", ", 2);
}

#endif
