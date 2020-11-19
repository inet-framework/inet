#include "inet/routing/ospfv3/process/Ospfv3RoutingTableEntry.h"

namespace inet {
namespace ospfv3 {

Ospfv3RoutingTableEntry::Ospfv3RoutingTableEntry(IInterfaceTable *ift, Ipv6Address destPrefix, int prefixLength, SourceType sourceType) :
    Ipv6Route(destPrefix, prefixLength, sourceType),
    ift(ift),
    area(BACKBONE_AREAID),
    pathType(Ospfv3RoutingTableEntry::INTRAAREA)
{
    this->setPrefixLength(prefixLength);
    this->setSourceType(IRoute::OSPF);
}

Ospfv3RoutingTableEntry::Ospfv3RoutingTableEntry(const Ospfv3RoutingTableEntry& entry, Ipv6Address destPrefix, int prefixLength, SourceType sourceType) :
    Ipv6Route(destPrefix, prefixLength, sourceType),
    destinationType(entry.destinationType),
    optionalCapabilities(entry.optionalCapabilities),
    area(entry.area),
    pathType(entry.pathType),
    cost(entry.cost),
    type2Cost(entry.type2Cost),
    linkStateOrigin(entry.linkStateOrigin),
    nextHops(entry.nextHops)
{
    this->setDestination(entry.getDestinationAsGeneric());
    this->setPrefixLength(prefixLength);
    this->setInterface(entry.getInterface());
    this->setSourceType(entry.getSourceType());
    this->setMetric(entry.getMetric());
}

void Ospfv3RoutingTableEntry::setPathType(RoutingPathType type)
{
    pathType = type;
    // FIXME this is a hack. But the correct way to do it is to implement a separate IIPv4RoutingTable module for OSPF...
    if (pathType == Ospfv3RoutingTableEntry::TYPE2_EXTERNAL) {
        setMetric(cost + type2Cost * 1000);
    }
    else {
        setMetric(cost);
    }
}

void Ospfv3RoutingTableEntry::setCost(Metric pathCost)
{
    cost = pathCost;
    // FIXME this is a hack. But the correct way to do it is to implement a separate IIPv4RoutingTable module for OSPF...
    if (pathType == Ospfv3RoutingTableEntry::TYPE2_EXTERNAL) {
        setMetric(cost + type2Cost * 1000);
    }
    else {
        setMetric(cost);
    }
}

void Ospfv3RoutingTableEntry::setType2Cost(Metric pathCost)
{
    type2Cost = pathCost;
    // FIXME this is a hack. But the correct way to do it is to implement a separate IIPv4RoutingTable module for OSPF...
    if (pathType == Ospfv3RoutingTableEntry::TYPE2_EXTERNAL) {
        setMetric(cost + type2Cost * 1000);
    }
    else {
        setMetric(cost);
    }
}

void Ospfv3RoutingTableEntry::addNextHop(NextHop hop)
{
    if (nextHops.size() == 0) {
        NetworkInterface *routingInterface = ift->getInterfaceById(hop.ifIndex);

        setInterface(routingInterface);
        // TODO this used to be commented out, but it seems we need it
        // otherwise gateways will never be filled in and gateway is needed for broadcast networks
//        setGateway(hop.hopAddress);
    }
    nextHops.push_back(hop);
}

bool Ospfv3RoutingTableEntry::operator==(const Ospfv3RoutingTableEntry& entry) const
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

    return (this->destinationType == entry.destinationType) &&
           (this->getDestinationAsGeneric() == entry.getDestinationAsGeneric()) &&
           (this->getPrefixLength() == entry.getPrefixLength()) &&
           (this->optionalCapabilities == entry.optionalCapabilities) &&
           (this->area == entry.area) &&
           (this->pathType == entry.pathType) &&
           (this->cost == entry.cost) &&
           (this->type2Cost == entry.type2Cost) &&
           (this->linkStateOrigin == entry.linkStateOrigin);
}

std::ostream& operator<<(std::ostream& out, const Ospfv3RoutingTableEntry& entry)
{
    out << "Destination: " << entry.getDestinationAsGeneric() << "/" << entry.getPrefixLength() << " (";
    if (entry.getDestinationType() == Ospfv3RoutingTableEntry::NETWORK_DESTINATION) {
        out << "Network";
    }
    else {
        if ((entry.getDestinationType() & Ospfv3RoutingTableEntry::AREA_BORDER_ROUTER_DESTINATION) != 0) {
            out << "AreaBorderRouter";
        }
        if ((entry.getDestinationType() & Ospfv3RoutingTableEntry::AS_BOUNDARY_ROUTER_DESTINATION) != 0) {
            if ((entry.getDestinationType() & Ospfv3RoutingTableEntry::AREA_BORDER_ROUTER_DESTINATION) != 0) {
                out << "+";
            }
            out << "ASBoundaryRouter";
        }
    }
    out << "), Area: "
        << entry.getArea().str(false)
        << ", PathType: ";
    switch (entry.getPathType()) {
        case Ospfv3RoutingTableEntry::INTRAAREA:
            out << "IntraArea";
            break;

        case Ospfv3RoutingTableEntry::INTERAREA:
            out << "InterArea";
            break;

        case Ospfv3RoutingTableEntry::TYPE1_EXTERNAL:
            out << "Type1External";
            break;

        case Ospfv3RoutingTableEntry::TYPE2_EXTERNAL:
            out << "Type2External";
            break;

        default:
            out << "Unknown";
            break;
    }
    out << ", iface: " << entry.getInterface()->getNedTypeName();
    out << ", Cost: " << entry.getCost()
        << ", Type2Cost: " << entry.getType2Cost()
        << ", Origin: ["; // << entry.getLinkStateOrigin()->getHeader().getLinkStateID()

    Ospfv3LsaHeader lsaHeader = entry.getLinkStateOrigin()->getHeader();
    out << "LSAHeader: age=" << lsaHeader.getLsaAge()
        << ", type=";
    switch (lsaHeader.getLsaType()) {
        case ROUTER_LSA:
            out << "RouterLSA";
            break;

        case NETWORK_LSA:
            out << "NetworkLSA";
            break;

        case INTER_AREA_PREFIX_LSA:
            out << "InterAreaPrefixLSA_Networks";
            break;

        case INTER_AREA_ROUTER_LSA:
            out << "InterAreaRouterLSA_ASBoundaryRouters";
            break;

        case AS_EXTERNAL_LSA:
            out << "ASExternalLSA";
            break;

        default:
            out << "Unknown";
            break;
    }
    out << ", LSID=" << lsaHeader.getLinkStateID().str(false)
        << ", advertisingRouter=" << lsaHeader.getAdvertisingRouter().str(false)
        << ", seqNumber=" << lsaHeader.getLsaSequenceNumber()
        << endl
        << "], NextHops: ";

    unsigned int hopCount = entry.getNextHopCount();
    for (unsigned int i = 0; i < hopCount; i++) {
        out << entry.getNextHop(i).hopAddress << "/" << entry.getNextHop(i).advertisingRouter << "/" << entry.getNextHop(i).ifIndex << "  ";
    }

    return out;
}

//-----------------------------------------------------------------------------------------------------------

Ospfv3Ipv4RoutingTableEntry::Ospfv3Ipv4RoutingTableEntry(IInterfaceTable *ift, Ipv4Address destPrefix, int prefixLength, SourceType sourceType) :
//     Ipv6Route(destPrefix, prefixLength, sourceType),
    ift(ift),
    area(BACKBONE_AREAID),
    pathType(Ospfv3Ipv4RoutingTableEntry::INTRAAREA)
{
    this->setDestination(destPrefix);
    this->setPrefixLength(prefixLength);
    this->setSourceType(sourceType);
}

Ospfv3Ipv4RoutingTableEntry::Ospfv3Ipv4RoutingTableEntry(const Ospfv3Ipv4RoutingTableEntry& entry, Ipv4Address destPrefix, int prefixLength, SourceType sourceType) :
    // Ipv6Route(destPrefix, prefixLength, sourceType),
    destinationType(entry.destinationType),
    optionalCapabilities(entry.optionalCapabilities),
    area(entry.area),
    pathType(entry.pathType),
    cost(entry.cost),
    type2Cost(entry.type2Cost),
    linkStateOrigin(entry.linkStateOrigin),
    nextHops(entry.nextHops)
{
    this->setDestination(entry.getDestinationAsGeneric());
    this->setPrefixLength(prefixLength);
    this->setInterface(entry.getInterface());
    this->setSourceType(entry.getSourceType());
    this->setMetric(entry.getMetric());
}

void Ospfv3Ipv4RoutingTableEntry::setPathType(RoutingPathType type)
{
    pathType = type;
    // FIXME this is a hack. But the correct way to do it is to implement a separate IIPv4RoutingTable module for OSPF...
    if (pathType == Ospfv3Ipv4RoutingTableEntry::TYPE2_EXTERNAL) {
        setMetric(cost + type2Cost * 1000);
    }
    else {
        setMetric(cost);
    }
}

void Ospfv3Ipv4RoutingTableEntry::setCost(Metric pathCost)
{
    cost = pathCost;
    // FIXME this is a hack. But the correct way to do it is to implement a separate IIPv4RoutingTable module for OSPF...
    if (pathType == Ospfv3Ipv4RoutingTableEntry::TYPE2_EXTERNAL) {
        setMetric(cost + type2Cost * 1000);
    }
    else {
        setMetric(cost);
    }
}

void Ospfv3Ipv4RoutingTableEntry::setType2Cost(Metric pathCost)
{
    type2Cost = pathCost;
    // FIXME this is a hack. But the correct way to do it is to implement a separate IIPv4RoutingTable module for OSPF...
    if (pathType == Ospfv3Ipv4RoutingTableEntry::TYPE2_EXTERNAL) {
        setMetric(cost + type2Cost * 1000);
    }
    else {
        setMetric(cost);
    }
}

void Ospfv3Ipv4RoutingTableEntry::addNextHop(NextHop hop)
{
    if (nextHops.size() == 0) {
        NetworkInterface *routingInterface = ift->getInterfaceById(hop.ifIndex);

        setInterface(routingInterface);
        // TODO this used to be commented out, but it seems we need it
        // otherwise gateways will never be filled in and gateway is needed for broadcast networks
//        setGateway(hop.hopAddress);
    }
    nextHops.push_back(hop);
}

bool Ospfv3Ipv4RoutingTableEntry::operator==(const Ospfv3Ipv4RoutingTableEntry& entry) const
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

    return (this->destinationType == entry.destinationType) &&
           (this->getDestinationAsGeneric() == entry.getDestinationAsGeneric()) &&
           (this->getPrefixLength() == entry.getPrefixLength()) &&
           (this->optionalCapabilities == entry.optionalCapabilities) &&
           (this->area == entry.area) &&
           (this->pathType == entry.pathType) &&
           (this->cost == entry.cost) &&
           (this->type2Cost == entry.type2Cost) &&
           (this->linkStateOrigin == entry.linkStateOrigin);
}

std::ostream& operator<<(std::ostream& out, const Ospfv3Ipv4RoutingTableEntry& entry)
{
    out << "Destination: " << entry.getDestinationAsGeneric() << "/" << entry.getPrefixLength() << " (";
    if (entry.getDestinationType() == Ospfv3Ipv4RoutingTableEntry::NETWORK_DESTINATION) {
        out << "Network";
    }
    else {
        if ((entry.getDestinationType() & Ospfv3Ipv4RoutingTableEntry::AREA_BORDER_ROUTER_DESTINATION) != 0) {
            out << "AreaBorderRouter";
        }
        if ((entry.getDestinationType() & Ospfv3Ipv4RoutingTableEntry::AS_BOUNDARY_ROUTER_DESTINATION) != 0) {
            if ((entry.getDestinationType() & Ospfv3Ipv4RoutingTableEntry::AREA_BORDER_ROUTER_DESTINATION) != 0) {
                out << "+";
            }
            out << "ASBoundaryRouter";
        }
    }
    out << "), Area: "
        << entry.getArea().str(false)
        << ", PathType: ";
    switch (entry.getPathType()) {
        case Ospfv3Ipv4RoutingTableEntry::INTRAAREA:
            out << "IntraArea";
            break;

        case Ospfv3Ipv4RoutingTableEntry::INTERAREA:
            out << "InterArea";
            break;

        case Ospfv3Ipv4RoutingTableEntry::TYPE1_EXTERNAL:
            out << "Type1External";
            break;

        case Ospfv3Ipv4RoutingTableEntry::TYPE2_EXTERNAL:
            out << "Type2External";
            break;

        default:
            out << "Unknown";
            break;
    }
    out << ", iface: " << entry.getInterface()->getNedTypeName();
    out << ", Cost: " << entry.getCost()
        << ", Type2Cost: " << entry.getType2Cost()
        << ", Origin: ["; // << entry.getLinkStateOrigin()->getHeader().getLinkStateID()

    Ospfv3LsaHeader lsaHeader = entry.getLinkStateOrigin()->getHeader();
    out << "LSAHeader: age=" << lsaHeader.getLsaAge()
        << ", type=";
    switch (lsaHeader.getLsaType()) {
        case ROUTER_LSA:
            out << "RouterLSA";
            break;

        case NETWORK_LSA:
            out << "NetworkLSA";
            break;

        case INTER_AREA_PREFIX_LSA:
            out << "InterAreaPrefixLSA_Networks";
            break;

        case INTER_AREA_ROUTER_LSA:
            out << "InterAreaRouterLSA_ASBoundaryRouters";
            break;

        case AS_EXTERNAL_LSA:
            out << "ASExternalLSA";
            break;

        default:
            out << "Unknown";
            break;
    }
    out << ", LSID=" << lsaHeader.getLinkStateID().str(false)
        << ", advertisingRouter=" << lsaHeader.getAdvertisingRouter().str(false)
        << ", seqNumber=" << lsaHeader.getLsaSequenceNumber()
        << endl
        << "], NextHops: ";

    unsigned int hopCount = entry.getNextHopCount();
    for (unsigned int i = 0; i < hopCount; i++) {
        out << entry.getNextHop(i).hopAddress << "/" << entry.getNextHop(i).advertisingRouter << "/" << entry.getNextHop(i).ifIndex << "  ";
    }

    return out;
}

} // namespace ospfv3
} // namespace inet

