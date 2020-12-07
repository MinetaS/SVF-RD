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
public:
    template<typename T>
    raw_ostream_debug &operator<<(T &val) {
        return RtDebugMode ? static_cast<raw_ostream_debug &>(static_cast<raw_ostream &>(*this) << val) : *this;
    }
};

#endif /* RD_GENERIC_H_ */