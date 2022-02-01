//
// Copyright (C) 2006 Andras Babos and Andras Varga
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_OSPFV2ROUTINGTABLEENTRY_H
#define __INET_OSPFV2ROUTINGTABLEENTRY_H

#include <memory.h>

#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/ipv4/IIpv4RoutingTable.h"
#include "inet/routing/ospfv2/Ospfv2Packet_m.h"
#include "inet/routing/ospfv2/router/Lsa.h"
#include "inet/routing/ospfv2/router/Ospfv2Common.h"

namespace inet {

namespace ospfv2 {

class INET_API Ospfv2RoutingTableEntry : public Ipv4Route
{
  public:
    enum RoutingPathType {
        INTRAAREA      = 0,
        INTERAREA      = 1,
        TYPE1_EXTERNAL = 2,
        TYPE2_EXTERNAL = 3
    };

    typedef unsigned char RoutingDestinationType;

    // destinationType bitfield values
    static const unsigned char NETWORK_DESTINATION = 0;
    static const unsigned char AREA_BORDER_ROUTER_DESTINATION = 1;
    static const unsigned char AS_BOUNDARY_ROUTER_DESTINATION = 2;

  private:
    IInterfaceTable *ift = nullptr;
    RoutingDestinationType destinationType = 0;
    Ospfv2Options optionalCapabilities;
    AreaId area;
    RoutingPathType pathType = static_cast<RoutingPathType>(-1);
    Metric cost = 0;
    Metric type2Cost = 0;
    const Ospfv2Lsa *linkStateOrigin = nullptr;
    std::vector<NextHop> nextHops;
    // Ipv4Route::interfacePtr comes from nextHops[0].ifIndex
    // Ipv4Route::gateway is nextHops[0].hopAddress

  public:
    Ospfv2RoutingTableEntry(IInterfaceTable *ift);
    Ospfv2RoutingTableEntry(const Ospfv2RoutingTableEntry& entry);
    virtual ~Ospfv2RoutingTableEntry() {}

    bool operator==(const Ospfv2RoutingTableEntry& entry) const;
    bool operator!=(const Ospfv2RoutingTableEntry& entry) const { return !((*this) == entry); }

    void setDestinationType(RoutingDestinationType type) { destinationType = type; }
    RoutingDestinationType getDestinationType() const { return destinationType; }
    void setOptionalCapabilities(Ospfv2Options options) { optionalCapabilities = options; }
    Ospfv2Options getOptionalCapabilities() const { return optionalCapabilities; }
    void setArea(AreaId source) { area = source; }
    AreaId getArea() const { return area; }
    void setPathType(RoutingPathType type) { pathType = type; }
    RoutingPathType getPathType() const { return pathType; }
    Metric getCost() const { return cost; }
    void setCost(Metric cost) { this->cost = cost; setMetric(cost); }
    void setType2Cost(Metric type2Cost) { this->type2Cost = type2Cost; setMetric(type2Cost); }
    Metric getType2Cost() const { return type2Cost; }
    void setLinkStateOrigin(const Ospfv2Lsa *lsa) { linkStateOrigin = lsa; }
    const Ospfv2Lsa *getLinkStateOrigin() const { return linkStateOrigin; }
    void addNextHop(NextHop hop);
    void clearNextHops() { nextHops.clear(); }
    unsigned int getNextHopCount() const { return nextHops.size(); }
    NextHop getNextHop(unsigned int index) const { return nextHops[index]; }
    virtual std::string str() const;

    static const char *getDestinationTypeString(RoutingDestinationType destType);
    static const char *getPathTypeString(RoutingPathType pathType);
};

std::ostream& operator<<(std::ostream& out, const Ospfv2RoutingTableEntry& entry);

} // namespace ospfv2

} // namespace inet

#endif

