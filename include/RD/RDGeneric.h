#ifndef RD_GENERIC_H_
#define RD_GENERIC_H_

#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"

using raw_ostream = llvm::raw_ostream;

class raw_ostream_debug;

extern bool RtDebugMode;
extern raw_ostream &out;
extern raw_ostream_debug &dout;

class raw_ostream_debug : raw_ostream {
#define __OVERRIDE_DEBUG_CAST_OPERATOR(v) (static_cast<raw_ostream_debug &>(static_cast<raw_ostream *>(this)->operator<<(v)))
public:
    template<typename T>
    raw_ostream_debug &operator<<(const T &val)  { return RtDebugMode ? __OVERRIDE_DEBUG_CAST_OPERATOR(val) : *this; }

    // prvalue support
    raw_ostream_debug &operator<<(const char val)               { return RtDebugMode ? __OVERRIDE_DEBUG_CAST_OPERATOR(val) : *this; }
    raw_ostream_debug &operator<<(const unsigned char val)      { return RtDebugMode ? __OVERRIDE_DEBUG_CAST_OPERATOR(val) : *this; }
    raw_ostream_debug &operator<<(const short val)              { return RtDebugMode ? __OVERRIDE_DEBUG_CAST_OPERATOR(val) : *this; }
    raw_ostream_debug &operator<<(const unsigned short val)     { return RtDebugMode ? __OVERRIDE_DEBUG_CAST_OPERATOR(val) : *this; }
    raw_ostream_debug &operator<<(const int val)                { return RtDebugMode ? __OVERRIDE_DEBUG_CAST_OPERATOR(val) : *this; }
    raw_ostream_debug &operator<<(const unsigned int val)       { return RtDebugMode ? __OVERRIDE_DEBUG_CAST_OPERATOR(val) : *this; }
    raw_ostream_debug &operator<<(const long val)               { return RtDebugMode ? __OVERRIDE_DEBUG_CAST_OPERATOR(val) : *this; }
    raw_ostream_debug &operator<<(const unsigned long val)      { return RtDebugMode ? __OVERRIDE_DEBUG_CAST_OPERATOR(val) : *this; }
    raw_ostream_debug &operator<<(const long long val)          { return RtDebugMode ? __OVERRIDE_DEBUG_CAST_OPERATOR(val) : *this; }
    raw_ostream_debug &operator<<(const unsigned long long val) { return RtDebugMode ? __OVERRIDE_DEBUG_CAST_OPERATOR(val) : *this; }
};

extern bool DisableAnalysis;

#endif /* RD_GENERIC_H_ */