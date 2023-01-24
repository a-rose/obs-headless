#pragma once

#include <libgen.h> // for basename(), not portable
#include <string>
#include <iostream>
#include <sstream>
#include <cstring>

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define TRACE_CONTEXT (std::string(__FILENAME__) + ":" + std::to_string(__LINE__) + " " + std::string(__func__) + "()")
#define ARRAY_LEN(a) ( sizeof(a)/sizeof((a)[0]) )

///////////////////////////////
// Trace format              //
///////////////////////////////

#define TRACE_FORMAT_NONE 0
#define TRACE_FORMAT_TEXT 1
#define TRACE_FORMAT_JSON 2

#ifndef APP_TRACE_FORMAT
#define APP_TRACE_FORMAT TRACE_FORMAT_TEXT
#endif

// When using TRACE_FORMAT_JSON
#ifndef APP_JSON_FLAGS
#define APP_JSON_FLAGS 0
#endif

///////////////////////////////
// Trace level               //
///////////////////////////////

enum trace_levels {
	TRACE_LEVEL_TRACE = 0,
	TRACE_LEVEL_DEBUG,
	TRACE_LEVEL_INFO,
	TRACE_LEVEL_WARN,
	TRACE_LEVEL_ERROR
};

static const char* trace_level_name[] = {
	"trace",
	"debug",
	"info",
	"warning",
	"error"
};

#ifndef APP_TRACE_LEVEL
#define APP_TRACE_LEVEL TRACE_LEVEL_INFO
#endif


///////////////////////////////
// Color                     //
///////////////////////////////

enum trace_colors {
	Default = 39,
	Black = 30,
	Red,
	Green,
	Yellow,
	Blue,
	Magenta,
	Cyan,
	LightGray,
	DarkGray = 90,
	LightRed,
	LightGreen,
	LightYellow,
	LightBlue,
	LightMagenta,
	LightCyan,
	White
};

#define ANSI_COLOR          "\x1b[%dm"
#define ANSI_COLOR_RESET    "\x1b[0m"


///////////////////////////////
// formatting helpers        //
///////////////////////////////

#define text_field(key, val) ("\x1b[37m"+ key + "=" ANSI_COLOR_RESET + val + "\x1b[37m" ANSI_COLOR_RESET)
#define json_field(key, val) (std::string("\"") + key + std::string("\": \"") + val + std::string("\""))

#define TO_STRING_STR(val) std::string(val)
#define TO_STRING_CHAR(val) std::string(val)
#define TO_STRING(val) std::to_string(val)

#define format_field(key, val) ( \
	(gTraceFormat == TRACE_FORMAT_TEXT) ? \
		text_field(key, val) \
	: (gTraceFormat == TRACE_FORMAT_JSON) ? \
		json_field(key, val) \
	: "" \
)

///////////////////////////////
// trace print (main macro)  //
///////////////////////////////

#define trace_print(level, color, message, ...) {\
	if(level >= gTraceLevel) {\
		if(gTraceFormat == TRACE_FORMAT_TEXT) { \
			std::string ar_[] = { __VA_ARGS__ }; \
			std::stringstream ss; \
			ss << "\x1b[" << std::to_string(color) << "m" \
				<< TRACE_CONTEXT << " " << trace_level_name[level] \
				<< ANSI_COLOR_RESET " " <<  message << "\t"; \
			for(int i=0; i<ARRAY_LEN(ar_) ; ++i){ \
				ss << ar_[i] << " "; \
			} \
			std::cout << ss.str() << std::endl; \
		} else if(gTraceFormat == TRACE_FORMAT_JSON) { \
			std::string ar_[] = { __VA_ARGS__ }; \
			std::stringstream ss; \
			ss << "\"level\": \"" << trace_level_name[level] << "\""; \
			ss << ", \"msg\": \"" << message << "\""; \
			if(level >= TRACE_LEVEL_ERROR) { \
				ss << ", \"context\": \"" << TRACE_CONTEXT << "\""; \
			} \
			for(int i=0; i<ARRAY_LEN(ar_) ; ++i){ \
				ss << ", " << ar_[i]; \
			} \
			std::cout << "{ " << ss.str() << " }" << std::endl; \
		} else if(gTraceFormat == TRACE_FORMAT_NONE) {\
				/* Disable all traces ! */ \
		} else { \
			printf("invalid or unset APP_TRACE_FORMAT\n"); \
		} \
	} \
}


///////////////////////////////
// fields macros             //
///////////////////////////////

// Use field_(n)s for std::string, field_(n)c for char*,
// field_(n)l for string litteral, and field(_n) for other types

#define field_n(key, val)	format_field(TO_STRING_CHAR(key), TO_STRING(val))
#define field_ns(key, val)	format_field(TO_STRING_CHAR(key), val)
#define field_nc(key, val)	format_field(TO_STRING_CHAR(key), TO_STRING_CHAR(val))
#define field_nl(key, val)	format_field(TO_STRING_CHAR(key), TO_STRING_STR(val))

#define field(val)			field_n(#val, val)
#define field_s(val)		field_ns(#val, val)
#define field_c(val)		field_nc(#val, val)
#define field_l(val)		field_ns(#val, val)
#define error(val)			field_ns("error", val)


///////////////////////////////
// per-level macros          //
///////////////////////////////

#define trace(msg, ...)			trace_print(TRACE_LEVEL_TRACE, LightYellow, msg, __VA_ARGS__)
#define trace_debug(msg, ...)	trace_print(TRACE_LEVEL_DEBUG, Blue,        msg, __VA_ARGS__)
#define trace_info(msg, ...)	trace_print(TRACE_LEVEL_INFO,  Green,       msg, __VA_ARGS__)
#define trace_warn(msg, ...)	trace_print(TRACE_LEVEL_WARN,  Yellow,      msg, __VA_ARGS__)
#define trace_error(msg, ...)	trace_print(TRACE_LEVEL_ERROR, Red,         msg, __VA_ARGS__)


///////////////////////////////
// Global variables          //
///////////////////////////////

extern int gTraceLevel;
extern int gTraceFormat;
