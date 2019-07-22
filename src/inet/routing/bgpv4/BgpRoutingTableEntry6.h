//    2019 Adrian Novak - multi address-family support

#ifndef __INET_BGPROUTINGTABLEENTRY6_H
#define __INET_BGPROUTINGTABLEENTRY6_H

#include "inet/networklayer/ipv6/Ipv6RoutingTable.h"
#include "inet/routing/bgpv4/BgpCommon.h"

namespace inet {

namespace bgp {

class INET_API RoutingTableEntry6 : public Ipv6Route
{
  public:
    typedef unsigned short RoutingPathType;

    RoutingTableEntry6(void);
    RoutingTableEntry6(const Ipv6Route *entry);
    virtual ~RoutingTableEntry6(void) {}

    void setPathType(RoutingPathType type) { _pathType = type; }
    RoutingPathType getPathType(void) const { return _pathType; }
    void addAS(AsId newAS) { _ASList.push_back(newAS); }
    unsigned int getASCount(void) const { return _ASList.size(); }
    AsId getAS(unsigned int index) const { return _ASList[index]; }

  private:
      // destinationID is RoutingEntry::host
      // addressMask is RoutingEntry::netmask
      RoutingPathType _pathType = INCOMPLETE;
      std::vector<AsId> _ASList;
};

inline RoutingTableEntry6::RoutingTableEntry6(void)
{
    setPrefixLength(128);
    setMetric(DEFAULT_COST);
    setSourceType(IRoute::BGP);
}

inline RoutingTableEntry6::RoutingTableEntry6(const Ipv6Route *entry)
{
    setDestination(entry->getDestPrefix());
    setPrefixLength(entry->getPrefixLength());
    setNextHop(entry->getNextHop());
    setInterface(entry->getInterface());
    setMetric(DEFAULT_COST);
    setSourceType(IRoute::BGP);
}

inline std::ostream& operator<<(std::ostream& out, RoutingTableEntry6& entry)
{
    out << "BGP - Destination: "
        << entry.getDestPrefix().str()
        << '/'
        << entry.getPrefixLength()
        << " , PathType: ";
    switch (entry.getPathType()) {
        case EGP:
            out << "EGP";
            break;

        case IGP:
            out << "IGP";
            break;

        case INCOMPLETE:
            out << "Incomplete";
            break;

        default:
            out << "Unknown";
            break;
    }

    out << " , NextHops: "
        << entry.getNextHop()
        << " , AS: ";
    unsigned int ASCount = entry.getASCount();
    for (unsigned int i = 0; i < ASCount; i++) {
        out << entry.getAS(i)
            << ' ';
    }
    return out;
}

} // namespace bgp

} // namespace inet

#endif // ifndef __INET_BGPROUTINGTABLEENTRY6_H
