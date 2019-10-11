#ifndef __INET_OSPFV3ROUTINGTABLEENTRY_H_
#define __INET_OSPFV3ROUTINGTABLEENTRY_H_

#include "inet/common/INETDefs.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/contract/IRoute.h"
#include "inet/networklayer/ipv4/Ipv4Route.h"
#include "inet/networklayer/ipv4/Ipv4RoutingTable.h"
#include "inet/networklayer/ipv6/Ipv6RoutingTable.h"
#include "inet/networklayer/ipv6/Ipv6Route.h"
#include "inet/routing/ospfv3/Ospfv3Common.h"
#include "inet/routing/ospfv3/Ospfv3Packet_m.h"

namespace inet {
namespace ospfv3 {

class INET_API Ospfv3RoutingTableEntry : public Ipv6Route
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
    Ospfv3Options optionalCapabilities;
    AreaID area;
    RoutingPathType pathType = (RoutingPathType)-1;
    Metric cost = 0;
    Metric type2Cost = 0;
    const Ospfv3Lsa *linkStateOrigin = nullptr;
    std::vector<NextHop> nextHops;
    // IPv4Route::interfacePtr comes from nextHops[0].ifIndex
    // IPv4Route::gateway is nextHops[0].hopAddress

public:
    Ospfv3RoutingTableEntry(const Ospfv3RoutingTableEntry& entry, Ipv6Address destPrefix, int prefixLength, SourceType sourceType);
    Ospfv3RoutingTableEntry(IInterfaceTable *ift, Ipv6Address destPrefix, int prefixLength, SourceType sourceType);
    virtual ~Ospfv3RoutingTableEntry() {}

    bool operator==(const Ospfv3RoutingTableEntry& entry) const;
    bool operator!=(const Ospfv3RoutingTableEntry& entry) const { return !((*this) == entry); }

    void setDestinationType(RoutingDestinationType type) { destinationType = type; }
    RoutingDestinationType getDestinationType() const { return destinationType; }
    void setOptionalCapabilities(Ospfv3Options options) { optionalCapabilities = options; }
    Ospfv3Options getOptionalCapabilities() const { return optionalCapabilities; }
    void setArea(AreaID source) { area = source; }
    AreaID getArea() const { return area; }
    void setPathType(RoutingPathType type);
    RoutingPathType getPathType() const { return pathType; }
    void setCost(Metric pathCost);
    Metric getCost() const { return cost; }
    void setType2Cost(Metric pathCost);
    Metric getType2Cost() const { return type2Cost; }
    void setLinkStateOrigin(const Ospfv3Lsa *lsa) { linkStateOrigin = lsa; }
    const Ospfv3Lsa *getLinkStateOrigin() const { return linkStateOrigin; }
    void addNextHop(NextHop hop);
    void clearNextHops() { nextHops.clear(); }
    unsigned int getNextHopCount() const { return nextHops.size(); }
    NextHop getNextHop(unsigned int index) const { return nextHops[index]; }
};

std::ostream& operator<<(std::ostream& out, const Ospfv3RoutingTableEntry& entry);

//------------------------------------------------------------------------------------------------------------
class INET_API Ospfv3Ipv4RoutingTableEntry : public Ipv4Route
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
    Ospfv3Options optionalCapabilities;
    AreaID area;
    RoutingPathType pathType = (RoutingPathType)-1;
    Metric cost = 0;
    Metric type2Cost = 0;
    const Ospfv3Lsa *linkStateOrigin = nullptr;
    std::vector<NextHop> nextHops;
    // IPv4Route::interfacePtr comes from nextHops[0].ifIndex
    // IPv4Route::gateway is nextHops[0].hopAddress

public:
    Ospfv3Ipv4RoutingTableEntry(const Ospfv3Ipv4RoutingTableEntry& entry, Ipv4Address destPrefix, int prefixLength, SourceType sourceType);
    Ospfv3Ipv4RoutingTableEntry(IInterfaceTable *ift, Ipv4Address destPrefix, int prefixLength, SourceType sourceType);
    virtual ~Ospfv3Ipv4RoutingTableEntry() {}

    bool operator==(const Ospfv3Ipv4RoutingTableEntry& entry) const;
    bool operator!=(const Ospfv3Ipv4RoutingTableEntry& entry) const { return !((*this) == entry); }

    void setDestinationType(RoutingDestinationType type) { destinationType = type; }
    RoutingDestinationType getDestinationType() const { return destinationType; }
    void setOptionalCapabilities(Ospfv3Options options) { optionalCapabilities = options; }
    Ospfv3Options getOptionalCapabilities() const { return optionalCapabilities; }
    void setArea(AreaID source) { area = source; }
    AreaID getArea() const { return area; }
    void setPathType(RoutingPathType type);
    RoutingPathType getPathType() const { return pathType; }
    void setCost(Metric pathCost);
    Metric getCost() const { return cost; }
    void setType2Cost(Metric pathCost);
    Metric getType2Cost() const { return type2Cost; }
    void setLinkStateOrigin(const Ospfv3Lsa *lsa) { linkStateOrigin = lsa; }
    const Ospfv3Lsa *getLinkStateOrigin() const { return linkStateOrigin; }
    void addNextHop(NextHop hop);
    void clearNextHops() { nextHops.clear(); }
    unsigned int getNextHopCount() const { return nextHops.size(); }
    NextHop getNextHop(unsigned int index) const { return nextHops[index]; }
};

std::ostream& operator<<(std::ostream& out, const Ospfv3Ipv4RoutingTableEntry& entry);

} // namespace ospfv3
}//namespace inet

#endif // __INET_OSPFV3ROUTINGTABLEENTRY_H_

