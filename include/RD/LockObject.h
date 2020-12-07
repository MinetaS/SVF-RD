#ifndef LOCKOBJECT_H_
#define LOCKOBJECT_H_

#include "SVF-FE/LLVMUtil.h"
#include "WPA/Andersen.h"

using namespace SVF;

class Lock;

using LockID = unsigned int;
using InstID = llvm::Instruction *;
using LockMap = std::unordered_map<std::string, LockID>;

#define LOCK_NONE (LockID)(0)

class Lock {
private:
    const LockID id;
    const InstID inst;
    const bool lock_type;   // lock:true, unlock:false

public:
    Lock(InstID inst, LockID id, bool type);
    Lock(InstID inst, std::string &name, bool type);

    static LockID getIDFromName(std::string &name);
    static LockID addObject(std::string &name, bool is_unlock = false);
};

#endif /* LOCKOBJECT_H_ */