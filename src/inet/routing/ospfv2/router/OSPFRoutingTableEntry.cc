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

#include "inet/routing/ospfv2/router/OSPFRoutingTableEntry.h"

namespace inet {

namespace ospf {

RoutingTableEntry::RoutingTableEntry(IInterfaceTable *_ift) :
    ift(_ift),
    destinationType(RoutingTableEntry::NETWORK_DESTINATION),
    area(BACKBONE_AREAID),
    pathType(RoutingTableEntry::INTRAAREA)
{
    setNetmask(IPv4Address::ALLONES_ADDRESS);
    setSourceType(IRoute::OSPF);
}

RoutingTableEntry::RoutingTableEntry(const RoutingTableEntry& entry) :
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

void RoutingTableEntry::setPathType(RoutingPathType type)
{
    pathType = type;
    // FIXME: this is a hack. But the correct way to do it is to implement a separate IIPv4RoutingTable module for OSPF...
    if (pathType == RoutingTableEntry::TYPE2_EXTERNAL) {
        setMetric(cost + type2Cost * 1000);
    }
    else {
        setMetric(cost);
    }
}

void RoutingTableEntry::setCost(Metric pathCost)
{
    cost = pathCost;
    // FIXME: this is a hack. But the correct way to do it is to implement a separate IIPv4RoutingTable module for OSPF...
    if (pathType == RoutingTableEntry::TYPE2_EXTERNAL) {
        setMetric(cost + type2Cost * 1000);
    }
    else {
        setMetric(cost);
    }
}

void RoutingTableEntry::setType2Cost(Metric pathCost)
{
    type2Cost = pathCost;
    // FIXME: this is a hack. But the correct way to do it is to implement a separate IIPv4RoutingTable module for OSPF...
    if (pathType == RoutingTableEntry::TYPE2_EXTERNAL) {
        setMetric(cost + type2Cost * 1000);
    }
    else {
        setMetric(cost);
    }
}

void RoutingTableEntry::addNextHop(NextHop hop)
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

bool RoutingTableEntry::operator==(const RoutingTableEntry& entry) const
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

std::ostream& operator<<(std::ostream& out, const RoutingTableEntry& entry)
{
    out << "Destination: " << entry.getDestination() << "/" << entry.getNetmask() << " (";
    if (entry.getDestinationType() == RoutingTableEntry::NETWORK_DESTINATION) {
        out << "Network";
    }
    else {
        if ((entry.getDestinationType() & RoutingTableEntry::AREA_BORDER_ROUTER_DESTINATION) != 0) {
            out << "AreaBorderRouter";
        }
        if ((entry.getDestinationType() & RoutingTableEntry::AS_BOUNDARY_ROUTER_DESTINATION) != 0) {
            if ((entry.getDestinationType() & RoutingTableEntry::AREA_BORDER_ROUTER_DESTINATION) != 0) {
                out << "+";
            }
            out << "ASBoundaryRouter";
        }
    }
    out << "), Area: "
        << entry.getArea().str(false)
        << ", PathType: ";
    switch (entry.getPathType()) {
        case RoutingTableEntry::INTRAAREA:
            out << "IntraArea";
            break;

        case RoutingTableEntry::INTERAREA:
            out << "InterArea";
            break;

        case RoutingTableEntry::TYPE1_EXTERNAL:
            out << "Type1External";
            break;

        case RoutingTableEntry::TYPE2_EXTERNAL:
            out << "Type2External";
            break;

        default:
            out << "Unknown";
            break;
    }
    out << ", iface: " << entry.getInterfaceName();
    out << ", Cost: " << entry.getCost()
        << ", Type2Cost: " << entry.getType2Cost()
        << ", Origin: [" << entry.getLinkStateOrigin()->getHeader()
        << "], NextHops: ";

    unsigned int hopCount = entry.getNextHopCount();
    for (unsigned int i = 0; i < hopCount; i++) {
        out << entry.getNextHop(i).hopAddress << " ";
    }

    return out;
}

} // namespace ospf

} // namespace inet

