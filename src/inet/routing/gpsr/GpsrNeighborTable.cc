//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/routing/gpsr/GpsrNeighborTable.h"

#include "inet/common/stlutils.h"

namespace inet {

std::vector<L3Address> GpsrNeighborTable::getAddresses() const
{
    std::vector<L3Address> addresses;
    for (const auto& elem : addressToPositionMap)
        addresses.push_back(elem.first);
    return addresses;
}

bool GpsrNeighborTable::hasPosition(const L3Address& address) const
{
    return containsKey(addressToPositionMap, address);
}

Coord GpsrNeighborTable::getPosition(const L3Address& address) const
{
    auto it = addressToPositionMap.find(address);
    return (it == addressToPositionMap.end()) ? Coord(NaN, NaN, NaN) : it->second.second;
}

void GpsrNeighborTable::setPosition(const L3Address& address, const Coord& coord)
{
    ASSERT(!address.isUnspecified());
    addressToPositionMap[address] = AddressToPositionMapValue(simTime(), coord);
}

void GpsrNeighborTable::removePosition(const L3Address& address)
{
    auto it = addressToPositionMap.find(address);
    addressToPositionMap.erase(it);
}

void GpsrNeighborTable::removeOldPositions(simtime_t timestamp)
{
    for (auto it = addressToPositionMap.begin(); it != addressToPositionMap.end();)
        if (it->second.first <= timestamp)
            addressToPositionMap.erase(it++);
        else
            it++;

}

void GpsrNeighborTable::clear()
{
    addressToPositionMap.clear();
}

simtime_t GpsrNeighborTable::getOldestPosition() const
{
    simtime_t oldestPosition = SimTime::getMaxTime();
    for (const auto& elem : addressToPositionMap) {
        const simtime_t& time = elem.second.first;
        if (time < oldestPosition)
            oldestPosition = time;
    }
    return oldestPosition;
}

std::ostream& operator<<(std::ostream& o, const GpsrNeighborTable& t)
{
    o << "{ ";
    for (auto elem : t.addressToPositionMap) {
        o << elem.first << ":(" << elem.second.first << ";" << elem.second.second << ") ";
    }
    o << "}";
    return o;
}

} // namespace inet

