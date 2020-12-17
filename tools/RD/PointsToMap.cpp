#include "RD/RDGeneric.h"
#include "RD/PointsToMap.h"


/* class PointsToMap */

void PointsToMap::initLoadSet(Pointer pt) {
    _M_map_load.insert(std::pair<Pointer, InstSet *>(pt, new InstSet()));
}

void PointsToMap::initStoreSet(Pointer pt) {
    _M_map_store.insert(std::pair<Pointer, InstSet *>(pt, new InstSet()));
}

bool PointsToMap::isLoadSetExists(Pointer pt) const {
    return _M_map_load.find(pt) != _M_map_load.end();
}

bool PointsToMap::isStoreSetExists(Pointer pt) const {
    return _M_map_store.find(pt) != _M_map_store.end();
}

InstSet *PointsToMap::getLoadSet(Pointer pt) const {
    __Internal_Map::const_iterator it = _M_map_load.find(pt);

    if (it == _M_map_load.end()) {
        return nullptr;
    }
    else {
        return it->second;
    }
}

InstSet *PointsToMap::getStoreSet(Pointer pt) const {
    __Internal_Map::const_iterator it = _M_map_store.find(pt);

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

PointsToMap::iterator PointsToMap::beginLoad() {
    return PointsToMap::iterator(_M_map_load.begin());
}

PointsToMap::iterator PointsToMap::endLoad() {
    return PointsToMap::iterator(_M_map_load.end());
}

PointsToMap::iterator PointsToMap::beginStore() {
    return PointsToMap::iterator(_M_map_store.begin());
}

PointsToMap::iterator PointsToMap::endStore() {
    return PointsToMap::iterator(_M_map_store.end());
}