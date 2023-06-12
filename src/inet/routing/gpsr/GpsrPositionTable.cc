//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/routing/gpsr/GpsrPositionTable.h"

#include "inet/common/stlutils.h"

namespace inet {

Coord GpsrPositionTable::getPosition(const L3Address& address) const
{
    auto it = addressToPositionMap.find(address);
    return (it == addressToPositionMap.end()) ? Coord::NIL : it->second.second;
}

void GpsrPositionTable::setPosition(const L3Address& address, const Coord& coord)
{
    ASSERT(!address.isUnspecified());
    addressToPositionMap[address] = AddressToPositionMapValue(simTime(), coord);
}

void GpsrPositionTable::clear()
{
    addressToPositionMap.clear();
}

std::ostream& operator<<(std::ostream& o, const GpsrPositionTable& t)
{
    o << "{ ";
    for (auto elem : t.addressToPositionMap) {
        o << elem.first << ":(" << elem.second.first << ";" << elem.second.second << ") ";
    }
    o << "}";
    return o;
}

} // namespace inet

