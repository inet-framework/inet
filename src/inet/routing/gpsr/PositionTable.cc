//
// Copyright (C) 2013 Opensim Ltd
// Author: Levente Meszaros
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

#include "inet/routing/gpsr/PositionTable.h"

namespace inet {

std::vector<L3Address> PositionTable::getAddresses() const
{
    std::vector<L3Address> addresses;
    for (AddressToPositionMap::const_iterator it = addressToPositionMap.begin(); it != addressToPositionMap.end(); it++)
        addresses.push_back(it->first);
    return addresses;
}

bool PositionTable::hasPosition(const L3Address& address) const
{
    AddressToPositionMap::const_iterator it = addressToPositionMap.find(address);
    return it != addressToPositionMap.end();
}

Coord PositionTable::getPosition(const L3Address& address) const
{
    AddressToPositionMap::const_iterator it = addressToPositionMap.find(address);
    if (it == addressToPositionMap.end())
        return Coord(NaN, NaN, NaN);
    else
        return it->second.second;
}

void PositionTable::setPosition(const L3Address& address, const Coord& coord)
{
    ASSERT(!address.isUnspecified());
    addressToPositionMap[address] = AddressToPositionMapValue(simTime(), coord);
}

void PositionTable::removePosition(const L3Address& address)
{
    AddressToPositionMap::iterator it = addressToPositionMap.find(address);
    addressToPositionMap.erase(it);
}

void PositionTable::removeOldPositions(simtime_t timestamp)
{
    for (AddressToPositionMap::iterator it = addressToPositionMap.begin(); it != addressToPositionMap.end(); )
        if (it->second.first <= timestamp)
            addressToPositionMap.erase(it++);
        else
            it++;

}

void PositionTable::clear()
{
    addressToPositionMap.clear();
}

simtime_t PositionTable::getOldestPosition() const
{
    simtime_t oldestPosition = SimTime::getMaxTime();
    for (AddressToPositionMap::const_iterator it = addressToPositionMap.begin(); it != addressToPositionMap.end(); it++) {
        const simtime_t& time = it->second.first;
        if (time < oldestPosition)
            oldestPosition = time;
    }
    return oldestPosition;
}

} // namespace inet

