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

class RoutingTableEntry : public IPRoute
{
public:
    enum RoutingPathType {
        IntraArea     = 0,
        InterArea     = 1,
        Type1External = 2,
        Type2External = 3
    };

    typedef unsigned char RoutingDestinationType;

    // destinationType bitfield values
    static const unsigned char NetworkDestination = 0;
    static const unsigned char AreaBorderRouterDestination = 1;
    static const unsigned char ASBoundaryRouterDestination = 2;

private:
    RoutingDestinationType  destinationType;
    // destinationID is IPRoute::host
    // addressMask is IPRoute::netmask
    OSPFOptions             optionalCapabilities;
    AreaID                  area;
    RoutingPathType         pathType;
    Metric                  cost;
    Metric                  type2Cost;
    const OSPFLSA*          linkStateOrigin;
    std::vector<NextHop>    nextHops;
    // IPRoute::interfacePtr comes from nextHops[0].ifIndex
    // IPRoute::gateway is nextHops[0].hopAddress

public:
            RoutingTableEntry  (void);
            RoutingTableEntry  (const RoutingTableEntry& entry);
    virtual ~RoutingTableEntry(void) {}

    bool    operator== (const RoutingTableEntry& entry) const;
    bool    operator!= (const RoutingTableEntry& entry) const { return (!((*this) == entry)); }

    void                    SetDestinationType      (RoutingDestinationType type)   { destinationType = type; }
    RoutingDestinationType  GetDestinationType      (void) const                    { return destinationType; }
    void                    SetDestinationID        (IPAddress destID)              { host = destID; }
    IPAddress               GetDestinationID        (void) const                    { return host; }
    void                    SetAddressMask          (IPAddress destMask)            { netmask = destMask; }
    IPAddress               GetAddressMask          (void) const                    { return netmask; }
    void                    SetOptionalCapabilities(OSPFOptions options)           { optionalCapabilities = options; }
    OSPFOptions             GetOptionalCapabilities(void) const                    { return optionalCapabilities; }
    void                    SetArea                 (AreaID source)                 { area = source; }
    AreaID                  GetArea                 (void) const                    { return area; }
    void                    SetPathType             (RoutingPathType type);
    RoutingPathType         GetPathType             (void) const                    { return pathType; }
    void                    SetCost                 (Metric pathCost);
    Metric                  GetCost                 (void) const                    { return cost; }
    void                    SetType2Cost            (Metric pathCost);
    Metric                  GetType2Cost            (void) const                    { return type2Cost; }
    void                    SetLinkStateOrigin      (const OSPFLSA* lsa)            { linkStateOrigin = lsa; }
    const OSPFLSA*          GetLinkStateOrigin      (void) const                    { return linkStateOrigin; }
    void                    AddNextHop              (NextHop hop);
    void                    ClearNextHops           (void)                          { nextHops.clear(); }
    unsigned int            GetNextHopCount         (void) const                    { return nextHops.size(); }
    NextHop                 GetNextHop              (unsigned int index) const      { return nextHops[index]; }
};

} // namespace OSPF

inline OSPF::RoutingTableEntry::RoutingTableEntry(void) :
    IPRoute(),
    destinationType(OSPF::RoutingTableEntry::NetworkDestination),
    area(OSPF::BackboneAreaID),
    pathType(OSPF::RoutingTableEntry::IntraArea),
    type2Cost(0),
    linkStateOrigin(NULL)
{
    netmask = 0xFFFFFFFF;
    source  = IPRoute::OSPF;
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
    host          = entry.host;
    netmask       = entry.netmask;
    gateway       = entry.gateway;
    interfacePtr  = entry.interfacePtr;
    type          = entry.type;
    source        = entry.source;
    metric        = entry.metric;
}

inline void OSPF::RoutingTableEntry::SetPathType(RoutingPathType type)
{
    pathType = type;
    // FIXME: this is a hack. But the correct way to do it is to implement a separate IRoutingTable module for OSPF...
    if (pathType == OSPF::RoutingTableEntry::Type2External) {
        metric = cost + type2Cost * 1000;
    } else {
        metric = cost;
    }
}

inline void OSPF::RoutingTableEntry::SetCost(Metric pathCost)
{
    cost = pathCost;
    // FIXME: this is a hack. But the correct way to do it is to implement a separate IRoutingTable module for OSPF...
    if (pathType == OSPF::RoutingTableEntry::Type2External) {
        metric = cost + type2Cost * 1000;
    } else {
        metric = cost;
    }
}

inline void OSPF::RoutingTableEntry::SetType2Cost(Metric pathCost)
{
    type2Cost = pathCost;
    // FIXME: this is a hack. But the correct way to do it is to implement a separate IRoutingTable module for OSPF...
    if (pathType == OSPF::RoutingTableEntry::Type2External) {
        metric = cost + type2Cost * 1000;
    } else {
        metric = cost;
    }
}

inline void OSPF::RoutingTableEntry::AddNextHop(OSPF::NextHop hop)
{
    if (nextHops.size() == 0) {
        InterfaceEntry*    routingInterface = InterfaceTableAccess().get()->getInterfaceById(hop.ifIndex);

        interfacePtr = routingInterface;
        //gateway = ULongFromIPv4Address(hop.hopAddress); // TODO: verify this isn't necessary
    }
    nextHops.push_back(hop);
}

inline bool OSPF::RoutingTableEntry::operator== (const RoutingTableEntry& entry) const
{
    unsigned int hopCount = nextHops.size();
    unsigned int i        = 0;

    if (hopCount != entry.nextHops.size()) {
        return false;
    }
    for (i = 0; i < hopCount; i++) {
        if ((nextHops[i] != entry.nextHops[i]))
        {
            return false;
        }
    }

    return ((destinationType      == entry.destinationType)      &&
            (host                 == entry.host)                 &&
            (netmask              == entry.netmask)              &&
            (optionalCapabilities == entry.optionalCapabilities) &&
            (area                 == entry.area)                 &&
            (pathType             == entry.pathType)             &&
            (cost                 == entry.cost)                 &&
            (type2Cost            == entry.type2Cost)            &&
            (linkStateOrigin      == entry.linkStateOrigin));
}

inline std::ostream& operator<< (std::ostream& out, const OSPF::RoutingTableEntry& entry)
{
    out << "Destination: "
        << entry.GetDestinationID().str()
        << "/"
        << entry.GetAddressMask().str()
        << " (";
    if (entry.GetDestinationType() == OSPF::RoutingTableEntry::NetworkDestination) {
        out << "Network";
    } else {
        if ((entry.GetDestinationType() & OSPF::RoutingTableEntry::AreaBorderRouterDestination) != 0) {
            out << "AreaBorderRouter";
        }
        if ((entry.GetDestinationType() & (OSPF::RoutingTableEntry::ASBoundaryRouterDestination | OSPF::RoutingTableEntry::AreaBorderRouterDestination)) != 0) {
            out << "+";
        }
        if ((entry.GetDestinationType() & OSPF::RoutingTableEntry::ASBoundaryRouterDestination) != 0) {
            out << "ASBoundaryRouter";
        }
    }
    out << "), Area: "
        << entry.GetArea()
        << ", PathType: ";
    switch (entry.GetPathType()) {
        case OSPF::RoutingTableEntry::IntraArea:     out << "IntraArea";     break;
        case OSPF::RoutingTableEntry::InterArea:     out << "InterArea";     break;
        case OSPF::RoutingTableEntry::Type1External: out << "Type1External"; break;
        case OSPF::RoutingTableEntry::Type2External: out << "Type2External"; break;
        default:            out << "Unknown";       break;
    }
    out << ", Cost: "
        << entry.GetCost()
        << ", Type2Cost: "
        << entry.GetType2Cost()
        << ", Origin: [";
    PrintLSAHeader(entry.GetLinkStateOrigin()->getHeader(), out);
    out << "], NextHops: ";

    unsigned int hopCount = entry.GetNextHopCount();
    for (unsigned int i = 0; i < hopCount; i++) {
        char addressString[16];
        out << AddressStringFromIPv4Address(addressString, sizeof(addressString), entry.GetNextHop(i).hopAddress)
            << " ";
    }

    return out;
}

#endif // __INET_OSPFROUTINGTABLEENTRY_H
