#ifndef LOCKOBJECT_H_
#define LOCKOBJECT_H_

#include "SVF-FE/LLVMUtil.h"
#include "WPA/Andersen.h"

#include <string>
#include <unordered_map>

using namespace SVF;

/* class declarations */
class LockFunctionMap;
class Lock;

/* type re-defenitions */
using std::string;
using std::unordered_map;

using FuncID = unsigned int;
using FuncType = unsigned int;
using FuncPair = std::pair<string, FuncID>;
using LockID = unsigned int;
using InstID = llvm::Instruction *;

/* constant values */
#define FUNC_NONE ((FuncID)0)
#define LOCK_NONE ((LockID)0)

#define TYPE_LOCK    ( 1)
#define TYPE_UNLOCK  ( 0)
#define TYPE_INVALID (-1)

/* class LockFunctionMap */
class LockFunctionMap {
private:
    unordered_map<string, FuncID> _M_map_name_to_id;

public:
    bool addObject(FuncID id, bool type, string &name);
    bool exists(string &name);
    FuncID getID(string &name);
    FuncType getType(string &name);

    /* ! Note
     * getID and getType internally uses exists therefore those methods
     * call std::unordered_map::find twice, but the time complexity is O(1)
     * so it won't affect the performance much.
     */
};

#define VAL_TO_INTERNAL(id, type) (((id) << 1) | (type))
#define INTERNAL_TO_VAL_ID(val)   ((val) >> 1)
#define INTERNAL_TO_VAL_TYPE(val) ((val) & 1)

#define CAST_UNION VAL_TO_INTERNAL
#define CAST_ID    INTERNAL_TO_VAL_ID
#define CAST_TYPE  INTERNAL_TO_VAL_TYPE

/* class Lock */
class Lock {
private:
    LockID _M_id;
    FuncID _M_fid;
    InstID _M_inst_lock;
    InstID _M_inst_unlock;

public:
    static void updateID();
    static void updateID(LockID __new_id);
    static LockID getLastID();

    Lock(LockID id, FuncID fid, InstID inst);
    Lock(LockID id, FuncID fid, InstID inst_lock, InstID inst_unlock);

    LockID getID() const;
    FuncID getFunctionID() const;
    InstID getLockInstruction() const;
    void setUnlockInstruction(InstID inst);
    InstID getUnlockInstruction() const;
};

#define INST_NONE ((InstID)nullptr)


#endif /* LOCKOBJECT_H_ */