
#ifndef WORKER_LOG_H
#define WORKER_LOG_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>

#define MAX_LOG_BUFFER_SIZE 100

typedef enum {

	INFO,
	DEBUG,
	ERROR,
	FATAL
} log_tag_t;



#define GEN_LOG_FUN(log_type, log_tag)\
	int log_##log_type(const char* fmt, ...){\
		char buff[MAX_LOG_BUFFER_SIZE];\
		char* curb = buff;\
		memset(buff, 0, sizeof(buff));\
		int ret = snprintf(curb, MAX_LOG_BUFFER_SIZE, "[%s] ", #log_tag);\
		curb += ret;\
		va_list ap;\
		va_start(ap, fmt);\
		ret = vsnprintf(curb, MAX_LOG_BUFFER_SIZE, fmt, ap);\
		va_end(ap);\
		fprintf(stdout, "%s", buff);\
		return ret;\
	}


int log_info(const char* fmt, ...);

#endif
