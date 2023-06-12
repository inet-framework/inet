//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_GPSRPOSITIONTABLE_H
#define __INET_GPSRPOSITIONTABLE_H

#include <map>
#include <vector>

#include "inet/common/geometry/common/Coord.h"
#include "inet/networklayer/common/L3Address.h"

namespace inet {

/**
 * This class provides a mapping between node addresses and their positions.
 */
class INET_API GpsrPositionTable
{
  private:
    typedef std::pair<simtime_t, Coord> AddressToPositionMapValue;
    typedef std::map<L3Address, AddressToPositionMapValue> AddressToPositionMap;
    AddressToPositionMap addressToPositionMap;

  public:
    Coord getPosition(const L3Address& address) const;
    void setPosition(const L3Address& address, const Coord& coord);

    void clear();

    friend std::ostream& operator<<(std::ostream& o, const GpsrPositionTable& t);
};

} // namespace inet

#endif

