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

#include "inet/routing/ospfv2/router/OspfRoutingTableEntry.h"

namespace inet {

namespace ospf {

OspfRoutingTableEntry::OspfRoutingTableEntry(IInterfaceTable *_ift) :
    ift(_ift),
    destinationType(OspfRoutingTableEntry::NETWORK_DESTINATION),
    area(BACKBONE_AREAID),
    pathType(OspfRoutingTableEntry::INTRAAREA)
{
    setNetmask(Ipv4Address::ALLONES_ADDRESS);
    setSourceType(IRoute::OSPF);
}

OspfRoutingTableEntry::OspfRoutingTableEntry(const OspfRoutingTableEntry& entry) :
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
    setSourceType(entry.getSourceType());
    setMetric(entry.getMetric());
}

void OspfRoutingTableEntry::setPathType(RoutingPathType type)
{
    pathType = type;
    // FIXME: this is a hack. But the correct way to do it is to implement a separate IIpv4RoutingTable module for OSPF...
    if (pathType == OspfRoutingTableEntry::TYPE2_EXTERNAL) {
        setMetric(cost + type2Cost * 1000);
    }
    else {
        setMetric(cost);
    }
}

void OspfRoutingTableEntry::setCost(Metric pathCost)
{
    cost = pathCost;
    // FIXME: this is a hack. But the correct way to do it is to implement a separate IIpv4RoutingTable module for OSPF...
    if (pathType == OspfRoutingTableEntry::TYPE2_EXTERNAL) {
        setMetric(cost + type2Cost * 1000);
    }
    else {
        setMetric(cost);
    }
}

void OspfRoutingTableEntry::setType2Cost(Metric pathCost)
{
    type2Cost = pathCost;
    // FIXME: this is a hack. But the correct way to do it is to implement a separate IIpv4RoutingTable module for OSPF...
    if (pathType == OspfRoutingTableEntry::TYPE2_EXTERNAL) {
        setMetric(cost + type2Cost * 1000);
    }
    else {
        setMetric(cost);
    }
}

void OspfRoutingTableEntry::addNextHop(NextHop hop)
{
    if (nextHops.size() == 0) {
        InterfaceEntry *routingInterface = ift->getInterfaceById(hop.ifIndex);

        setInterface(routingInterface);
        // TODO: this used to be commented out, but it seems we need it
        // otherwise gateways will never be filled in and gateway is needed for broadcast networks
        setGateway(hop.hopAddress);
    }
    nextHops.push_back(hop);
}

bool OspfRoutingTableEntry::operator==(const OspfRoutingTableEntry& entry) const
{
    unsigned int hopCount = nextHops.size();
    unsigned int i = 0;

    if (hopCount != entry.nextHops.size()) {
        return false;
    }
    for (i = 0; i < hopCount; i++) {
        if ((nextHops[i] != entry.nextHops[i])) {
            return false;
        }
    }

    return (destinationType == entry.destinationType) &&
           (getDestination() == entry.getDestination()) &&
           (getNetmask() == entry.getNetmask()) &&
           (optionalCapabilities == entry.optionalCapabilities) &&
           (area == entry.area) &&
           (pathType == entry.pathType) &&
           (cost == entry.cost) &&
           (type2Cost == entry.type2Cost) &&
           (linkStateOrigin == entry.linkStateOrigin);
}

std::ostream& operator<<(std::ostream& out, const OspfRoutingTableEntry& entry)
{
    out << "dest: " << entry.getDestination() << " ";
    out << "nextHops: ";
    for (unsigned int i = 0; i < entry.getNextHopCount(); i++) {
        Ipv4Address gateway = entry.getNextHop(i).hopAddress;
        if (gateway.isUnspecified())
            out << "*  ";
        else
            out << gateway << "  ";
    }
    out << "mask: " << entry.getNetmask() << " ";
    out << "cost: " << entry.getCost() << " ";
    out << "if: " << entry.getInterfaceName() << " ";
    out << "destType: " << OspfRoutingTableEntry::getDestinationTypeString(entry.getDestinationType());
    out << " area: " << entry.getArea().str(false) << " ";
    out << "pathType: " << OspfRoutingTableEntry::getPathTypeString(entry.getPathType());
    out << " Type2Cost: " << entry.getType2Cost() << " ";
    out << "Origin: [" << entry.getLinkStateOrigin()->getHeader() << "] ";

    return out;
}


const std::string OspfRoutingTableEntry::getDestinationTypeString(RoutingDestinationType destType)
{
    std::stringstream out;

    if (destType == OspfRoutingTableEntry::NETWORK_DESTINATION) {
        out << "Network";
    }
    else {
        if ((destType & OspfRoutingTableEntry::AREA_BORDER_ROUTER_DESTINATION) != 0) {
            out << "AreaBorderRouter";
        }
        if ((destType & OspfRoutingTableEntry::AS_BOUNDARY_ROUTER_DESTINATION) != 0) {
            if ((destType & OspfRoutingTableEntry::AREA_BORDER_ROUTER_DESTINATION) != 0) {
                out << "+";
            }
            out << "ASBoundaryRouter";
        }
    }

    return out.str();
}

const std::string OspfRoutingTableEntry::getPathTypeString(RoutingPathType pathType)
{
    switch (pathType) {
        case OspfRoutingTableEntry::INTRAAREA:
            return "IntraArea";

        case OspfRoutingTableEntry::INTERAREA:
            return "InterArea";

        case OspfRoutingTableEntry::TYPE1_EXTERNAL:
            return "Type1External";

        case OspfRoutingTableEntry::TYPE2_EXTERNAL:
            return "Type2External";
    }

    return "Unknown";
}

} // namespace ospf

} // namespace inet

