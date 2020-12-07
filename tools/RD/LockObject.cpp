#include "RD/RDGeneric.h"
#include "RD/LockObject.h"

#include <unordered_map>

static LockMap locks;
static LockID last = 1;

Lock::Lock(InstID inst, LockID id, bool type)
: inst(inst), id(id), lock_type(type) {
    dout << "Lock object created (inst=" << inst << ", id=" << id << ")\n";
}

Lock::Lock(InstID inst, std::string &name, bool type)
: Lock(inst, getIDFromName(name), type) {

}

LockID Lock::getIDFromName(std::string &name) {
    LockMap::iterator __r = locks.find(name);

    if (__r == locks.end()) {
        return LOCK_NONE;
    }

    return __r->second;
}

LockID Lock::addObject(std::string &name, bool is_unlock) {
    if (LockID __id = getIDFromName(name)) {
        return __id;
    }

    std::string __s_lock, __s_unlock;
    size_t __t;

    if (!is_unlock) {
        __t = name.find("lock");
        assert (__t != std::string::npos);
        __s_lock = name;
        __s_unlock = name.substr(0, __t) + "un" + name.substr(__t);
    }
    else {
        __t = name.find("unlock");
        assert (__t != std::string::npos);
        __s_lock = name.substr(0, __t) + name.substr(__t+2);
        __s_unlock = name;
    }

    locks.insert(pair<std::string, LockID>(__s_lock, last));
    locks.insert(pair<std::string, LockID>(__s_unlock, last));
    return last++;
}