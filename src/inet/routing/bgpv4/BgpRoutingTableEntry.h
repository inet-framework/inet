//
// Copyright (C) 2010 Helene Lageber
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_BGPROUTINGTABLEENTRY_H
#define __INET_BGPROUTINGTABLEENTRY_H

#include "inet/networklayer/contract/IRoute.h"
#include "inet/networklayer/ipv4/Ipv4RoutingTable.h"
#include "inet/networklayer/ipv6/Ipv6Route.h"
#include "inet/routing/bgpv4/BgpCommon.h"

namespace inet {

namespace bgp {

// BGP per-route attributes, mixed into an address-family-specific route so a single object
// can live both in the BGP RIB (bgpRoutingTable) and in the system routing table. The
// decision process holds BgpRouteInfo* and reaches the underlying route via asRoute() (or
// the generic forwarders below), so it is address-family agnostic; concrete entries are
// BgpRoutingTableEntry (over Ipv4Route) and BgpRoutingTableEntry6 (over Ipv6Route).
class INET_API BgpRouteInfo
{
  public:
    enum { DEFAULT_COST = 1 };
    typedef unsigned char RoutingPathType;

  protected:
    RoutingPathType _pathType = INCOMPLETE;
    std::vector<AsId> _ASList;
    int localPreference = 0;
    bool iBgpLearned = false;

  public:
    virtual ~BgpRouteInfo() {}

    // the underlying address-family route (this object, viewed as an IRoute)
    virtual IRoute *asRoute() = 0;
    virtual const IRoute *asRoute() const = 0;

    // BGP attributes
    void setPathType(RoutingPathType type) { _pathType = type; }
    RoutingPathType getPathType() const { return _pathType; }
    static const std::string getPathTypeString(RoutingPathType type);
    void addAS(AsId newAs) { _ASList.push_back(newAs); }
    unsigned int getASCount() const { return _ASList.size(); }
    AsId getAS(unsigned int index) const { return _ASList[index]; }
    int getLocalPreference() const { return localPreference; }
    void setLocalPreference(int l) { localPreference = l; }
    bool isIBgpLearned() const { return iBgpLearned; }
    void setIBgpLearned(bool i) { iBgpLearned = i; }

    // generic route accessors, forwarded to the underlying IRoute
    L3Address getDestinationAsGeneric() const { return asRoute()->getDestinationAsGeneric(); }
    void setDestination(const L3Address& dest) { asRoute()->setDestination(dest); }
    int getPrefixLength() const { return asRoute()->getPrefixLength(); }
    void setPrefixLength(int len) { asRoute()->setPrefixLength(len); }
    L3Address getNextHopAsGeneric() const { return asRoute()->getNextHopAsGeneric(); }
    void setNextHop(const L3Address& nextHop) { asRoute()->setNextHop(nextHop); }
    NetworkInterface *getInterface() const { return asRoute()->getInterface(); }
    void setInterface(NetworkInterface *ie) { asRoute()->setInterface(ie); }
    IRoute::SourceType getSourceType() const { return asRoute()->getSourceType(); }
    void setSourceType(IRoute::SourceType type) { asRoute()->setSourceType(type); }
    int getMetric() const { return asRoute()->getMetric(); }
    void setMetric(int m) { asRoute()->setMetric(m); }
    void setAdminDist(unsigned int d) { asRoute()->setAdminDist(d); }

    std::string bgpStr() const;
};

// IPv4 BGP RIB entry: an Ipv4Route carrying BGP attributes.
class INET_API BgpRoutingTableEntry : public Ipv4Route, public BgpRouteInfo
{
  public:
    BgpRoutingTableEntry();
    BgpRoutingTableEntry(const IRoute *entry);
    virtual ~BgpRoutingTableEntry() {}
    virtual IRoute *asRoute() override { return this; }
    virtual const IRoute *asRoute() const override { return this; }
    virtual std::string str() const override { return bgpStr(); }
};

inline BgpRoutingTableEntry::BgpRoutingTableEntry()
{
    Ipv4Route::setNetmask(Ipv4Address::ALLONES_ADDRESS);
    Ipv4Route::setMetric(DEFAULT_COST);
    Ipv4Route::setSourceType(IRoute::BGP);
}

inline BgpRoutingTableEntry::BgpRoutingTableEntry(const IRoute *entry)
{
    Ipv4Route::setDestination(entry->getDestinationAsGeneric().toIpv4());
    Ipv4Route::setPrefixLength(entry->getPrefixLength());
    Ipv4Route::setNextHop(entry->getNextHopAsGeneric());
    Ipv4Route::setInterface(entry->getInterface());
    Ipv4Route::setMetric(DEFAULT_COST);
    Ipv4Route::setSourceType(IRoute::BGP);
    Ipv4Route::setAdminDist(Ipv4Route::dBGPExternal);
}

// IPv6 BGP RIB entry: an Ipv6Route carrying BGP attributes.
class INET_API BgpRoutingTableEntry6 : public Ipv6Route, public BgpRouteInfo
{
  public:
    BgpRoutingTableEntry6();
    BgpRoutingTableEntry6(const IRoute *entry);
    virtual ~BgpRoutingTableEntry6() {}
    virtual IRoute *asRoute() override { return this; }
    virtual const IRoute *asRoute() const override { return this; }
    virtual std::string str() const override { return bgpStr(); }
};

inline BgpRoutingTableEntry6::BgpRoutingTableEntry6()
    : Ipv6Route(Ipv6Address(), 128, IRoute::BGP)
{
    Ipv6Route::setMetric(DEFAULT_COST);
}

inline BgpRoutingTableEntry6::BgpRoutingTableEntry6(const IRoute *entry)
    : Ipv6Route(entry->getDestinationAsGeneric().toIpv6(), entry->getPrefixLength(), IRoute::BGP)
{
    Ipv6Route::setNextHop(entry->getNextHopAsGeneric());
    Ipv6Route::setInterface(entry->getInterface());
    Ipv6Route::setMetric(DEFAULT_COST);
    Ipv6Route::setAdminDist(Ipv6Route::dBGPExternal);
}

inline const std::string BgpRouteInfo::getPathTypeString(RoutingPathType type)
{
    if (type == IGP)
        return "IGP";
    else if (type == EGP)
        return "EGP";
    else if (type == INCOMPLETE)
        return "INCOMPLETE";

    return "Unknown";
}

inline std::string BgpRouteInfo::bgpStr() const
{
    std::stringstream out;
    out << "BGP ";
    if (getDestinationAsGeneric().isUnspecified())
        out << "*";
    else
        out << getDestinationAsGeneric();
    out << "/" << getPrefixLength();
    out << " gw:";
    if (getNextHopAsGeneric().isUnspecified())
        out << "*  ";
    else
        out << getNextHopAsGeneric() << "  ";
    out << "metric:" << getMetric() << "  ";
    out << "if:" << (getInterface() ? getInterface()->getInterfaceName() : "*");
    out << " origin: " << getPathTypeString(_pathType);
    if (iBgpLearned)
        out << " localPref: " << localPreference;
    out << " ASlist: ";
    for (auto& element : _ASList)
        out << element << ' ';

    return out.str();
}

inline std::ostream& operator<<(std::ostream& out, const BgpRouteInfo& entry)
{
    if (entry.getDestinationAsGeneric().isUnspecified())
        out << "*";
    else
        out << entry.getDestinationAsGeneric();
    out << "/" << entry.getPrefixLength()
        << " nextHop: " << entry.getNextHopAsGeneric()
        << " cost: " << entry.getMetric()
        << " if: " << (entry.getInterface() ? entry.getInterface()->getInterfaceName() : "*")
        << " origin: " << BgpRouteInfo::getPathTypeString(entry.getPathType());
    if (entry.isIBgpLearned())
        out << " localPref: " << entry.getLocalPreference();
    out << " ASlist: ";
    for (unsigned int i = 0; i < entry.getASCount(); i++)
        out << entry.getAS(i) << ' ';

    return out;
}

} // namespace bgp

} // namespace inet

#endif
