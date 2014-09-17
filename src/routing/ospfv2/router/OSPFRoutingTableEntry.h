//
// Copyright (C) 2006 Andras Babos and Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_OSPFROUTINGTABLEENTRY_H
#define __INET_OSPFROUTINGTABLEENTRY_H

#include <memory.h>

#include "inet/common/INETDefs.h"

#include "inet/networklayer/common/IInterfaceTable.h"
#include "inet/networklayer/ipv4/IIPv4RoutingTable.h"
#include "inet/routing/ospfv2/router/LSA.h"
#include "inet/routing/ospfv2/router/OSPFcommon.h"
#include "inet/routing/ospfv2/OSPFPacket_m.h"

namespace inet {

namespace ospf {

class RoutingTableEntry : public IPv4Route
{
  public:
    enum RoutingPathType {
        INTRAAREA = 0,
        INTERAREA = 1,
        TYPE1_EXTERNAL = 2,
        TYPE2_EXTERNAL = 3
    };

    typedef unsigned char RoutingDestinationType;

    // destinationType bitfield values
    static const unsigned char NETWORK_DESTINATION = 0;
    static const unsigned char AREA_BORDER_ROUTER_DESTINATION = 1;
    static const unsigned char AS_BOUNDARY_ROUTER_DESTINATION = 2;

  private:
    IInterfaceTable *ift;
    RoutingDestinationType destinationType;
    OSPFOptions optionalCapabilities;
    AreaID area;
    RoutingPathType pathType;
    Metric cost;
    Metric type2Cost;
    const OSPFLSA *linkStateOrigin;
    std::vector<NextHop> nextHops;
    // IPv4Route::interfacePtr comes from nextHops[0].ifIndex
    // IPv4Route::gateway is nextHops[0].hopAddress

  public:
    RoutingTableEntry(IInterfaceTable *ift);
    RoutingTableEntry(const RoutingTableEntry& entry);
    virtual ~RoutingTableEntry() {}

    bool operator==(const RoutingTableEntry& entry) const;
    bool operator!=(const RoutingTableEntry& entry) const { return !((*this) == entry); }

    void setDestinationType(RoutingDestinationType type) { destinationType = type; }
    RoutingDestinationType getDestinationType() const { return destinationType; }
    void setOptionalCapabilities(OSPFOptions options) { optionalCapabilities = options; }
    OSPFOptions getOptionalCapabilities() const { return optionalCapabilities; }
    void setArea(AreaID source) { area = source; }
    AreaID getArea() const { return area; }
    void setPathType(RoutingPathType type);
    RoutingPathType getPathType() const { return pathType; }
    void setCost(Metric pathCost);
    Metric getCost() const { return cost; }
    void setType2Cost(Metric pathCost);
    Metric getType2Cost() const { return type2Cost; }
    void setLinkStateOrigin(const OSPFLSA *lsa) { linkStateOrigin = lsa; }
    const OSPFLSA *getLinkStateOrigin() const { return linkStateOrigin; }
    void addNextHop(NextHop hop);
    void clearNextHops() { nextHops.clear(); }
    unsigned int getNextHopCount() const { return nextHops.size(); }
    NextHop getNextHop(unsigned int index) const { return nextHops[index]; }
};

std::ostream& operator<<(std::ostream& out, const RoutingTableEntry& entry);

} // namespace ospf

} // namespace inet

#endif // ifndef __INET_OSPFROUTINGTABLEENTRY_H

