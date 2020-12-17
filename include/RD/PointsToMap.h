#ifndef POINTSTOMAP_H_
#define POINTSTOMAP_H_

#include "MSSA/MemSSA.h"

#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace SVF;

using InstID = llvm::Instruction *;
using InstSet = std::unordered_set<InstID>;
using Pointer = unsigned int;


class PointsToMap {
private:
    using __Internal_Map = std::unordered_map<Pointer, InstSet *>;
    using __Internal_Pair = std::pair<Pointer, InstSet *>;

    __Internal_Map _M_map_load;
    __Internal_Map _M_map_store;

    void initLoadSet(Pointer pt);
    void initStoreSet(Pointer pt);

public:
    class iterator;

    bool isLoadSetExists(Pointer pt) const;
    bool isStoreSetExists(Pointer pt) const;
    InstSet *getLoadSet(Pointer pt) const;
    InstSet *getStoreSet(Pointer pt) const;
    void addInstructionToLoadSet(Pointer pts, InstID inst);
    void addInstructionToStoreSet(Pointer pts, InstID inst);
    iterator beginLoad();
    iterator beginStore();
    iterator endLoad();
    iterator endStore();

    class iterator {
    private:
        __Internal_Map::iterator _M_ptr;

    public:
        iterator(__Internal_Map::iterator ptr) : _M_ptr(ptr) {}

        iterator operator++()      { ++_M_ptr; return *this; }
        iterator operator++(int _) { iterator i = *this; ++_M_ptr; return i; }
        const __Internal_Pair operator*() { return *_M_ptr; }
        const __Internal_Map::iterator operator->() { return _M_ptr; }
        bool operator==(const iterator &r) { return _M_ptr == r._M_ptr; }
        bool operator!=(const iterator &r) { return _M_ptr != r._M_ptr; }

        Pointer getPoint() const { return _M_ptr->first; }
        InstSet *getSet() const { return _M_ptr->second; }
    };
};


#endif /* POINTSTOMAP_H_ */