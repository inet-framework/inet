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

#ifndef __INET_POSITIONTABLE_H_
#define __INET_POSITIONTABLE_H_

#include <vector>
#include <map>
#include "INETDefs.h"
#include "IPvXAddress.h"
#include "Coord.h"

/**
 * This class provides a mapping between node addresses and their positions.
 */
class INET_API PositionTable {
    private:
        typedef std::pair<simtime_t, Coord> AddressToPositionMapValue;
        typedef std::map<IPvXAddress, AddressToPositionMapValue> AddressToPositionMap;
        AddressToPositionMap addressToPositionMap;

    public:
        PositionTable() { }

        std::vector<IPvXAddress> getAddresses() const;

        bool hasPosition(const IPvXAddress & address) const;
        Coord getPosition(const IPvXAddress & address) const;
        void setPosition(const IPvXAddress & address, const Coord & coord);

        void removePosition(const IPvXAddress & address);
        void removeOldPositions(simtime_t timestamp);

        void clear();

        simtime_t getOldestPosition() const;
};

#endif
