#include "RD/RDGeneric.h"
#include "RD/LockObject.h"


/* class LockFunctionMap */

bool LockFunctionMap::addObject(FuncID id, bool type, string &name) {
    if (_M_map_name_to_id.find(name) != _M_map_name_to_id.end()) {
        return false;
    }

    _M_map_name_to_id.insert(FuncPair(name, CAST_UNION(id, type)));
    dout << "  Function map entry registered: id=" << id << ", name=" << name << ", type=" << type << "\n";

    return true;
}

bool LockFunctionMap::exists(string &name) {
    return _M_map_name_to_id.find(name) != _M_map_name_to_id.cend();
}

FuncID LockFunctionMap::getID(string &name) {
    if (exists(name)) {
        return CAST_ID(_M_map_name_to_id.find(name)->second);
    }
    else {
        return FUNC_NONE;
    }
}

FuncType LockFunctionMap::getType(string &name) {
    if (exists(name)) {
        return CAST_TYPE(_M_map_name_to_id.find(name)->second);
    }
    else {
        return FUNC_NONE;
    }
}

/* class Lock */

LockID LastID = 1;

void Lock::updateID() {
    ++LastID;
}

void Lock::updateID(LockID __new_val) {
    LastID = __new_val;
}

LockID Lock::getLastID() {
    return LastID;
}

Lock::Lock(LockID id, FuncID fid, InstID inst)
: _M_id(id), _M_fid(fid), _M_inst_lock(inst), _M_inst_unlock(INST_NONE) {
    if (RtDebugMode) {
        dout << "  Lock object created (id=" << id << ", fid=" << fid << ", inst_lock=" << inst << ")\n";
    }
}

Lock::Lock(LockID id, FuncID fid, InstID inst_lock, InstID inst_unlock)
: _M_id(id), _M_fid(fid), _M_inst_lock(inst_lock), _M_inst_unlock(inst_unlock) {
    if (RtDebugMode) {
        dout << "  Lock object created (id=" << id << ", fid=" << fid << ", inst_lock=" << inst_lock << ", inst_unlock=" << inst_unlock << ")\n";
    }
}

LockID Lock::getID() const {
    return _M_id;
}

FuncID Lock::getFunctionID() const {
    return _M_fid;
}

InstID Lock::getLockInstruction() const {
    return _M_inst_lock;
}

void Lock::setUnlockInstruction(InstID inst) {
    _M_inst_unlock = inst;
}

InstID Lock::getUnlockInstruction() const {
    return _M_inst_unlock;
}