//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_GPSRNEIGHBORTABLE_H
#define __INET_GPSRNEIGHBORTABLE_H

#include <map>
#include <vector>

#include "inet/common/geometry/common/Coord.h"
#include "inet/networklayer/common/L3Address.h"

namespace inet {

/**
 * This class provides a mapping between node addresses and their positions.
 */
class INET_API GpsrNeighborTable
{
  private:
    struct Neighbor {
        int networkInterfaceId = -1;
        Coord position = Coord::NIL;
        simtime_t lastUpdate = -1;

        Neighbor() {}
        Neighbor(int networkInterfaceId, const Coord& position, simtime_t lastUpdate) :
            networkInterfaceId(networkInterfaceId), position(position), lastUpdate(lastUpdate) {}
    };

    std::map<L3Address, Neighbor> addressToNeighborMap;

  public:
    GpsrNeighborTable() {}

    std::vector<L3Address> getAddresses() const;

    bool hasNeighbor(const L3Address& address) const;
    int getNetworkInterfaceId(const L3Address& address) const;
    Coord getPosition(const L3Address& address) const;
    void updateNeighbor(const L3Address& address, int networkInterfaceId, const Coord& position);

    void removeNeighbor(const L3Address& address);
    void removeOldNeighbors(simtime_t timestamp);

    void clear();

    simtime_t getOldestNeighbor() const;

    friend std::ostream& operator<<(std::ostream& o, const GpsrNeighborTable& t);
};

} // namespace inet

#endif

