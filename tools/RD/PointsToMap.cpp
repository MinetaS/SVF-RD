#include "RD/RDGeneric.h"
#include "RD/PointsToMap.h"


/* class PointsToMap */

void PointsToMap::initLoadSet(Pointer pt) {
    _M_map_load.insert(std::pair<Pointer, InstSet *>(pt, new InstSet()));
}

void PointsToMap::initStoreSet(Pointer pt) {
    _M_map_store.insert(std::pair<Pointer, InstSet *>(pt, new InstSet()));
}

bool PointsToMap::isLoadSetExists(Pointer pt) {
    return _M_map_load.find(pt) != _M_map_load.end();
}

bool PointsToMap::isStoreSetExists(Pointer pt) {
    return _M_map_store.find(pt) != _M_map_store.end();
}

InstSet *PointsToMap::getLoadSet(Pointer pt) {
    __Internal_Map::iterator it = _M_map_load.find(pt);

    if (it == _M_map_load.end()) {
        return nullptr;
    }
    else {
        return it->second;
    }
}

InstSet *PointsToMap::getStoreSet(Pointer pt) {
    __Internal_Map::iterator it = _M_map_store.find(pt);

    if (it == _M_map_store.end()) {
        return nullptr;
    }
    else {
        return it->second;
    }
}

void PointsToMap::addInstructionToLoadSet(Pointer pt, InstID inst) {
    if (!isLoadSetExists(pt)) {
        initLoadSet(pt);
    }

    getLoadSet(pt)->insert(inst);
}

void PointsToMap::addInstructionToStoreSet(Pointer pt, InstID inst) {
    if (!isStoreSetExists(pt)) {
        initStoreSet(pt);
    }

    getStoreSet(pt)->insert(inst);
}