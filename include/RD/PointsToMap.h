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
using __Internal_Map = std::unordered_map<Pointer, InstSet *>;


class PointsToMap {
private:
    __Internal_Map _M_map_load;
    __Internal_Map _M_map_store;

    void initLoadSet(Pointer pt);
    void initStoreSet(Pointer pt);

public:
    bool isLoadSetExists(Pointer pt);
    bool isStoreSetExists(Pointer pt);
    InstSet *getLoadSet(Pointer pt);
    InstSet *getStoreSet(Pointer pt);
    void addInstructionToLoadSet(Pointer pts, InstID inst);
    void addInstructionToStoreSet(Pointer pts, InstID inst);
};


#endif /* POINTSTOMAP_H_ */