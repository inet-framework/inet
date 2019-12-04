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

#include "inet/routing/ospfv2/router/Ospfv2RoutingTableEntry.h"

namespace inet {

namespace ospfv2 {

Ospfv2RoutingTableEntry::Ospfv2RoutingTableEntry(IInterfaceTable *_ift) :
    ift(_ift),
    destinationType(Ospfv2RoutingTableEntry::NETWORK_DESTINATION),
    area(BACKBONE_AREAID),
    pathType(Ospfv2RoutingTableEntry::INTRAAREA)
{
    setNetmask(Ipv4Address::ALLONES_ADDRESS);
    setSourceType(IRoute::OSPF);
    setAdminDist(Ipv4Route::dOSPF);
}

Ospfv2RoutingTableEntry::Ospfv2RoutingTableEntry(const Ospfv2RoutingTableEntry& entry) :
    ift(entry.ift),
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
    setInterface(entry.getInterface());
    setGateway(entry.getGateway());
    setSourceType(entry.getSourceType());
    setMetric(entry.getMetric());
    setAdminDist(entry.getAdminDist());
}

void Ospfv2RoutingTableEntry::addNextHop(NextHop hop)
{
    if (nextHops.size() == 0) {
        InterfaceEntry *routingInterface = ift->getInterfaceById(hop.ifIndex);

        setInterface(routingInterface);
        // TODO: this used to be commented out, but it seems we need it
        // otherwise gateways will never be filled in and gateway is needed for broadcast networks
        setGateway(hop.hopAddress);
    }else{
      for(int i= 0; i < nextHops.size(); i++){
        if(hop.ifIndex == nextHops.at(i).ifIndex && hop.hopAddress == nextHops.at(i).hopAddress){
          return;
        }
      }
    }
    nextHops.push_back(hop);
}

bool Ospfv2RoutingTableEntry::operator==(const Ospfv2RoutingTableEntry& entry) const
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

std::ostream& operator<<(std::ostream& out, const Ospfv2RoutingTableEntry& entry)
{
    if (entry.getDestination().isUnspecified())
        out << "0.0.0.0";
    else
        out << entry.getDestination();
    out << "/";
    if (entry.getNetmask().isUnspecified())
        out << "0";
    else
        out << entry.getNetmask().getNetmaskLength();
    out << " nextHops: ";
    for (unsigned int i = 0; i < entry.getNextHopCount(); i++) {
        Ipv4Address gateway = entry.getNextHop(i).hopAddress;
        if (gateway.isUnspecified())
            out << "*  ";
        else
            out << gateway << "  ";
    }
    out << "cost: " << entry.getCost() << " ";
    if(entry.getPathType() == Ospfv2RoutingTableEntry::TYPE2_EXTERNAL)
        out << "type2Cost: " << entry.getType2Cost() << " ";
    out << "if: " << entry.getInterfaceName() << " ";
    out << "destType: " << Ospfv2RoutingTableEntry::getDestinationTypeString(entry.getDestinationType());
    out << " area: " << entry.getArea().str(false) << " ";
    out << "pathType: " << Ospfv2RoutingTableEntry::getPathTypeString(entry.getPathType()) << " ";
    out << "Origin: [" << entry.getLinkStateOrigin()->getHeader() << "] ";

    return out;
}

std::string Ospfv2RoutingTableEntry::str() const
{
    std::stringstream out;
    out << getSourceTypeAbbreviation();
    out << " ";
    if (getDestination().isUnspecified())
        out << "0.0.0.0";
    else
        out << getDestination();
    out << "/";
    if (getNetmask().isUnspecified())
        out << "0";
    else
        out << getNetmask().getNetmaskLength();
    out << " gw:";
    if (getGateway().isUnspecified())
        out << "*  ";
    else
        out << getGateway() << "  ";
    if(getRoutingTable() && getRoutingTable()->isAdminDistEnabled())
        out << "AD:" << getAdminDist() << "  ";
    out << "metric:" << getMetric() << "  ";
    out << "if:";
    if (!getInterface())
        out << "*";
    else
        out << getInterfaceName();

    out << " destType:" << Ospfv2RoutingTableEntry::getDestinationTypeString(destinationType)
    << " pathType:" << Ospfv2RoutingTableEntry::getPathTypeString(pathType)
    << " area:" << area.str(false);

    return out.str();
}

const std::string Ospfv2RoutingTableEntry::getDestinationTypeString(RoutingDestinationType destType)
{
    std::stringstream out;

    if (destType == Ospfv2RoutingTableEntry::NETWORK_DESTINATION) {
        out << "Network";
    }
    else {
        if ((destType & Ospfv2RoutingTableEntry::AREA_BORDER_ROUTER_DESTINATION) != 0) {
            out << "AreaBorderRouter";
        }
        if ((destType & Ospfv2RoutingTableEntry::AS_BOUNDARY_ROUTER_DESTINATION) != 0) {
            if ((destType & Ospfv2RoutingTableEntry::AREA_BORDER_ROUTER_DESTINATION) != 0) {
                out << "+";
            }
            out << "ASBoundaryRouter";
        }
    }

    return out.str();
}

const std::string Ospfv2RoutingTableEntry::getPathTypeString(RoutingPathType pathType)
{
    switch (pathType) {
        case Ospfv2RoutingTableEntry::INTRAAREA:
            return "IntraArea";

        case Ospfv2RoutingTableEntry::INTERAREA:
            return "InterArea";

        case Ospfv2RoutingTableEntry::TYPE1_EXTERNAL:
            return "Type1External";

        case Ospfv2RoutingTableEntry::TYPE2_EXTERNAL:
            return "Type2External";
    }

    return "Unknown";
}

} // namespace ospfv2

} // namespace inet

