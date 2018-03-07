#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

#include "scu_internal.h"

void _scu_output_module_list(int fd, const char *modulename);
void _scu_output_test_list(int fd, size_t index, const char *name, const char *description, const char *tags[]);
void _scu_output_module_start(int fd, const char *modulename);
void _scu_output_module_end(int fd);
void _scu_output_setup_start(int fd, const char *filename);
void _scu_output_setup_end(int fd);
void _scu_output_teardown_start(int fd, const char *filename);
void _scu_output_teardown_end(int fd);
void _scu_output_test_start(int fd, int idx, const char *name, const char *filename);
void _scu_output_test_failure(int fd, _scu_failure *failure);
void _scu_output_test_failures(int fd, size_t num, _scu_failure *failures);
void _scu_output_test_end(int fd, int idx, bool success, size_t asserts, double mono_time,
                          double cpu_time, size_t num_failures, _scu_failure failures[]);

#endif
