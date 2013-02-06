//
// Copyright (C) 2004 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#include "PositionTable.h"

std::vector<Address> PositionTable::getAddresses() const {
    std::vector<Address> addresses;
    for (AddressToPositionMap::const_iterator it = addressToPositionMap.begin(); it != addressToPositionMap.end(); it++)
        addresses.push_back(it->first);
    return addresses;
}

Coord PositionTable::getPosition(const Address & address) const {
    AddressToPositionMap::const_iterator it = addressToPositionMap.find(address);
    if (it == addressToPositionMap.end())
        return Coord();
    else
        return it->second.second;
}

void PositionTable::setPosition(const Address & address, const Coord & coord) {
    ASSERT(!address.isUnspecified());
    addressToPositionMap[address] = AddressToPositionMapValue(simTime(), coord);
}

void PositionTable::removePosition(const Address & address) {
    AddressToPositionMap::iterator it = addressToPositionMap.find(address);
    addressToPositionMap.erase(it);
}

void PositionTable::removeOldPositions(simtime_t timestamp) {
    for (AddressToPositionMap::iterator it = addressToPositionMap.begin(); it != addressToPositionMap.end();)
        if (it->second.first <= timestamp)
            addressToPositionMap.erase(it++);
        else
            it++;
}

simtime_t PositionTable::getOldestPosition() {
    simtime_t oldestPosition = SimTime::getMaxTime();
    for (AddressToPositionMap::iterator it = addressToPositionMap.begin(); it != addressToPositionMap.end(); it++) {
        const simtime_t & time = it->second.first;
        if (time < oldestPosition)
            oldestPosition = time;
    }
    return oldestPosition;
}
