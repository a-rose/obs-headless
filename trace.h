#include <libgen.h> // for basename(), not portable
#include <string>

#pragma once

enum trace_colors
{
	Red = 31,
	Green,
	Yellow,
	Blue,
	Magenta,
	Cyan,
	Normal
};

#define ANSI_COLOR    		"\x1b[%dm"
#define ANSI_COLOR_RESET   	"\x1b[0m"

#define trace_print(color, fmt, args...) { 	\
	std::string filename = __FILE__; 		\
	printf(ANSI_COLOR "%s:%d %s()| " ANSI_COLOR_RESET fmt "\n", color, basename((char*) filename.c_str()), __LINE__, __func__, ##args); \
}

#define trace(fmt, args...) { 	\
	trace_print(Blue, fmt, ##args);		\
}

#define trace_error(fmt, args...) { 	\
	trace_print(Red, fmt, ##args);		\
}
