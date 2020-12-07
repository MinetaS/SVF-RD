#include "RD/RDGeneric.h"

bool RtDebugMode = false;
raw_ostream &out = llvm::outs();
raw_ostream_debug &dout = (raw_ostream_debug &)out;