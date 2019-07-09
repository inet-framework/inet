#ifndef __INET_OSPFV3ROUTINGTABLEENTRY_H_
#define __INET_OSPFV3ROUTINGTABLEENTRY_H_

#include <omnetpp.h>

#include "inet/routing/ospfv3/OSPFv3Common.h"
#include "inet/routing/ospfv3/OSPFv3Packet_m.h"
#include "inet/common/INETDefs.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/ipv6/Ipv6RoutingTable.h"
#include "inet/networklayer/ipv6/Ipv6Route.h"
#include "inet/networklayer/contract/IRoute.h"
#include "inet/networklayer/ipv4/Ipv4RoutingTable.h"
#include "inet/networklayer/ipv4/Ipv4Route.h"

namespace inet {

class INET_API OSPFv3RoutingTableEntry : public Ipv6Route
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
    IInterfaceTable *ift = nullptr;
    RoutingDestinationType destinationType = 0;
    OSPFv3Options optionalCapabilities;
    AreaID area;
    RoutingPathType pathType = (RoutingPathType)-1;
    Metric cost = 0;
    Metric type2Cost = 0;
    const OSPFv3LSA *linkStateOrigin = nullptr;
    std::vector<NextHop> nextHops;
    // IPv4Route::interfacePtr comes from nextHops[0].ifIndex
    // IPv4Route::gateway is nextHops[0].hopAddress

public:
    OSPFv3RoutingTableEntry(const OSPFv3RoutingTableEntry& entry, Ipv6Address destPrefix, int prefixLength, SourceType sourceType);
    OSPFv3RoutingTableEntry(IInterfaceTable *ift, Ipv6Address destPrefix, int prefixLength, SourceType sourceType);
    virtual ~OSPFv3RoutingTableEntry() {}

    bool operator==(const OSPFv3RoutingTableEntry& entry) const;
    bool operator!=(const OSPFv3RoutingTableEntry& entry) const { return !((*this) == entry); }

    void setDestinationType(RoutingDestinationType type) { destinationType = type; }
    RoutingDestinationType getDestinationType() const { return destinationType; }
    void setOptionalCapabilities(OSPFv3Options options) { optionalCapabilities = options; }
    OSPFv3Options getOptionalCapabilities() const { return optionalCapabilities; }
    void setArea(AreaID source) { area = source; }
    AreaID getArea() const { return area; }
    void setPathType(RoutingPathType type);
    RoutingPathType getPathType() const { return pathType; }
    void setCost(Metric pathCost);
    Metric getCost() const { return cost; }
    void setType2Cost(Metric pathCost);
    Metric getType2Cost() const { return type2Cost; }
    void setLinkStateOrigin(const OSPFv3LSA *lsa) { linkStateOrigin = lsa; }
    const OSPFv3LSA *getLinkStateOrigin() const { return linkStateOrigin; }
    void addNextHop(NextHop hop);
    void clearNextHops() { nextHops.clear(); }
    unsigned int getNextHopCount() const { return nextHops.size(); }
    NextHop getNextHop(unsigned int index) const { return nextHops[index]; }
};

std::ostream& operator<<(std::ostream& out, const OSPFv3RoutingTableEntry& entry);


//------------------------------------------------------------------------------------------------------------
class INET_API OSPFv3IPv4RoutingTableEntry : public Ipv4Route
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
    static const unsigned char ROUTER_DESTINATION = 3;

private:
    IInterfaceTable *ift = nullptr;
    RoutingDestinationType destinationType = 0;
    OSPFv3Options optionalCapabilities;
    AreaID area;
    RoutingPathType pathType = (RoutingPathType)-1;
    Metric cost = 0;
    Metric type2Cost = 0;
    const OSPFv3LSA *linkStateOrigin = nullptr;
    std::vector<NextHop> nextHops;
    // IPv4Route::interfacePtr comes from nextHops[0].ifIndex
    // IPv4Route::gateway is nextHops[0].hopAddress

public:
    OSPFv3IPv4RoutingTableEntry(const OSPFv3IPv4RoutingTableEntry& entry, Ipv4Address destPrefix, int prefixLength, SourceType sourceType);
    OSPFv3IPv4RoutingTableEntry(IInterfaceTable *ift, Ipv4Address destPrefix, int prefixLength, SourceType sourceType);
    virtual ~OSPFv3IPv4RoutingTableEntry() {}

    bool operator==(const OSPFv3IPv4RoutingTableEntry& entry) const;
    bool operator!=(const OSPFv3IPv4RoutingTableEntry& entry) const { return !((*this) == entry); }

    void setDestinationType(RoutingDestinationType type) { destinationType = type; }
    RoutingDestinationType getDestinationType() const { return destinationType; }
    void setOptionalCapabilities(OSPFv3Options options) { optionalCapabilities = options; }
    OSPFv3Options getOptionalCapabilities() const { return optionalCapabilities; }
    void setArea(AreaID source) { area = source; }
    AreaID getArea() const { return area; }
    void setPathType(RoutingPathType type);
    RoutingPathType getPathType() const { return pathType; }
    void setCost(Metric pathCost);
    Metric getCost() const { return cost; }
    void setType2Cost(Metric pathCost);
    Metric getType2Cost() const { return type2Cost; }
    void setLinkStateOrigin(const OSPFv3LSA *lsa) { linkStateOrigin = lsa; }
    const OSPFv3LSA *getLinkStateOrigin() const { return linkStateOrigin; }
    void addNextHop(NextHop hop);
    void clearNextHops() { nextHops.clear(); }
    unsigned int getNextHopCount() const { return nextHops.size(); }
    NextHop getNextHop(unsigned int index) const { return nextHops[index]; }
};

std::ostream& operator<<(std::ostream& out, const OSPFv3IPv4RoutingTableEntry& entry);

}//namespace inet

#endif
