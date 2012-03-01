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

#include "IRoutingTable.h"
#include "InterfaceTableAccess.h"
#include "OSPFcommon.h"
#include <memory.h>

namespace OSPF {

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
    RoutingDestinationType  destinationType;
    OSPFOptions             optionalCapabilities;
    AreaID                  area;
    RoutingPathType         pathType;
    Metric                  cost;
    Metric                  type2Cost;
    const OSPFLSA*          linkStateOrigin;
    std::vector<NextHop>    nextHops;
    // IPv4Route::interfacePtr comes from nextHops[0].ifIndex
    // IPv4Route::gateway is nextHops[0].hopAddress

public:
    RoutingTableEntry();
    RoutingTableEntry(const RoutingTableEntry& entry);
    virtual ~RoutingTableEntry() {}

    bool operator==(const RoutingTableEntry& entry) const;
    bool operator!=(const RoutingTableEntry& entry) const { return (!((*this) == entry)); }

    void                   setDestinationType(RoutingDestinationType type)  { destinationType = type; }
    RoutingDestinationType getDestinationType() const  { return destinationType; }
    void                   setOptionalCapabilities(OSPFOptions options)  { optionalCapabilities = options; }
    OSPFOptions            getOptionalCapabilities() const  { return optionalCapabilities; }
    void                   setArea(AreaID source)  { area = source; }
    AreaID                 getArea() const  { return area; }
    void                   setPathType(RoutingPathType type);
    RoutingPathType        getPathType() const  { return pathType; }
    void                   setCost(Metric pathCost);
    Metric                 getCost() const  { return cost; }
    void                   setType2Cost(Metric pathCost);
    Metric                 getType2Cost() const  { return type2Cost; }
    void                   setLinkStateOrigin(const OSPFLSA* lsa)  { linkStateOrigin = lsa; }
    const OSPFLSA*         getLinkStateOrigin() const  { return linkStateOrigin; }
    void                   addNextHop(NextHop hop);
    void                   clearNextHops()  { nextHops.clear(); }
    unsigned int           getNextHopCount() const  { return nextHops.size(); }
    NextHop                getNextHop(unsigned int index) const  { return nextHops[index]; }
};

} // namespace OSPF

inline OSPF::RoutingTableEntry::RoutingTableEntry() :
    IPv4Route(),
    destinationType(OSPF::RoutingTableEntry::NETWORK_DESTINATION),
    area(OSPF::BACKBONE_AREAID),
    pathType(OSPF::RoutingTableEntry::INTRAAREA),
    type2Cost(0),
    linkStateOrigin(NULL)
{
    setNetmask(0xFFFFFFFF);
    setSource(IPv4Route::OSPF);
    memset(&optionalCapabilities, 0, sizeof(OSPFOptions));
}

inline OSPF::RoutingTableEntry::RoutingTableEntry(const RoutingTableEntry& entry) :
    destinationType(entry.destinationType),
    optionalCapabilities(entry.optionalCapabilities),
    area(entry.area),
    pathType(entry.pathType),
    cost(entry.cost),
    type2Cost(entry.type2Cost),
    linkStateOrigin(entry.linkStateOrigin),
    nextHops(entry.nextHops)
{
    setDestination(entry.getDestination());
    setNetmask(entry.getNetmask());
    setGateway(entry.getGateway());
    setInterface(entry.getInterface());
    setType(entry.getType());
    setSource(entry.getSource());
    setMetric(entry.getMetric());
}

inline void OSPF::RoutingTableEntry::setPathType(RoutingPathType type)
{
    pathType = type;
    // FIXME: this is a hack. But the correct way to do it is to implement a separate IRoutingTable module for OSPF...
    if (pathType == OSPF::RoutingTableEntry::TYPE2_EXTERNAL) {
        setMetric(cost + type2Cost * 1000);
    } else {
        setMetric(cost);
    }
}

inline void OSPF::RoutingTableEntry::setCost(Metric pathCost)
{
    cost = pathCost;
    // FIXME: this is a hack. But the correct way to do it is to implement a separate IRoutingTable module for OSPF...
    if (pathType == OSPF::RoutingTableEntry::TYPE2_EXTERNAL) {
        setMetric(cost + type2Cost * 1000);
    } else {
        setMetric(cost);
    }
}

inline void OSPF::RoutingTableEntry::setType2Cost(Metric pathCost)
{
    type2Cost = pathCost;
    // FIXME: this is a hack. But the correct way to do it is to implement a separate IRoutingTable module for OSPF...
    if (pathType == OSPF::RoutingTableEntry::TYPE2_EXTERNAL) {
        setMetric(cost + type2Cost * 1000);
    } else {
        setMetric(cost);
    }
}

inline void OSPF::RoutingTableEntry::addNextHop(OSPF::NextHop hop)
{
    if (nextHops.size() == 0) {
        InterfaceEntry*    routingInterface = InterfaceTableAccess().get()->getInterfaceById(hop.ifIndex);

        setInterface(routingInterface);
        // TODO: this used to be commented out, but it seems we need it
        // otherwise gateways will never be filled in and gateway is needed for broadcast networks
        setGateway(ulongFromIPv4Address(hop.hopAddress));
    }
    nextHops.push_back(hop);
}

inline bool OSPF::RoutingTableEntry::operator==(const RoutingTableEntry& entry) const
{
    unsigned int hopCount = nextHops.size();
    unsigned int i = 0;

    if (hopCount != entry.nextHops.size()) {
        return false;
    }
    for (i = 0; i < hopCount; i++) {
        if ((nextHops[i] != entry.nextHops[i]))
        {
            return false;
        }
    }

    return ((destinationType == entry.destinationType) &&
            (getDestination() == entry.getDestination()) &&
            (getNetmask() == entry.getNetmask()) &&
            (optionalCapabilities == entry.optionalCapabilities) &&
            (area == entry.area) &&
            (pathType == entry.pathType) &&
            (cost == entry.cost) &&
            (type2Cost == entry.type2Cost) &&
            (linkStateOrigin == entry.linkStateOrigin));
}

inline std::ostream& operator<<(std::ostream& out, const OSPF::RoutingTableEntry& entry)
{
    out << "Destination: "
        << entry.getDestination().str()
        << "/"
        << entry.getNetmask().str()
        << " (";
    if (entry.getDestinationType() == OSPF::RoutingTableEntry::NETWORK_DESTINATION) {
        out << "Network";
    } else {
        if ((entry.getDestinationType() & OSPF::RoutingTableEntry::AREA_BORDER_ROUTER_DESTINATION) != 0) {
            out << "AreaBorderRouter";
        }
        if ((entry.getDestinationType() & (OSPF::RoutingTableEntry::AS_BOUNDARY_ROUTER_DESTINATION | OSPF::RoutingTableEntry::AREA_BORDER_ROUTER_DESTINATION)) != 0) {
            out << "+";
        }
        if ((entry.getDestinationType() & OSPF::RoutingTableEntry::AS_BOUNDARY_ROUTER_DESTINATION) != 0) {
            out << "ASBoundaryRouter";
        }
    }
    out << "), Area: "
        << entry.getArea()
        << ", PathType: ";
    switch (entry.getPathType()) {
        case OSPF::RoutingTableEntry::INTRAAREA:      out << "IntraArea";     break;
        case OSPF::RoutingTableEntry::INTERAREA:      out << "InterArea";     break;
        case OSPF::RoutingTableEntry::TYPE1_EXTERNAL: out << "Type1External"; break;
        case OSPF::RoutingTableEntry::TYPE2_EXTERNAL: out << "Type2External"; break;
        default:                                      out << "Unknown";       break;
    }
    out << ", Cost: "
        << entry.getCost()
        << ", Type2Cost: "
        << entry.getType2Cost()
        << ", Origin: [";
    printLSAHeader(entry.getLinkStateOrigin()->getHeader(), out);
    out << "], NextHops: ";

    unsigned int hopCount = entry.getNextHopCount();
    for (unsigned int i = 0; i < hopCount; i++) {
        char addressString[16];
        out << addressStringFromIPv4Address(addressString, sizeof(addressString), entry.getNextHop(i).hopAddress)
            << " ";
    }

    return out;
}

#endif // __INET_OSPFROUTINGTABLEENTRY_H
