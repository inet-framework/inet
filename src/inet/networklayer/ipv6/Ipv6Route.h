//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IPV6ROUTE_H
#define __INET_IPV6ROUTE_H

#include <vector>

#include "inet/networklayer/contract/IRoute.h"
#include "inet/networklayer/contract/ipv6/Ipv6Address.h"

namespace inet {

class NetworkInterface;
class Ipv6RoutingTable;

/**
 * Represents a route in the route table. Routes with sourceType=ROUTER_ADVERTISEMENT represent
 * on-link prefixes advertised by routers.
 */
class INET_API Ipv6Route : public cObject, public IRoute
{
  public:
    /** Cisco like administrative distances (includes Ipv4 protocols)*/
    enum RouteAdminDist {
        dDirectlyConnected = 0,
        dStatic            = 1,
        dEIGRPSummary      = 5,
        dBGPExternal       = 20,
        dEIGRPInternal     = 90,
        dIGRP              = 100,
        dOSPF              = 110,
        dISIS              = 115,
        dRIP               = 120,
        dEGP               = 140,
        dODR               = 160,
        dEIGRPExternal     = 170,
        dBGPInternal       = 200,
        dDHCPlearned       = 254,
        dBABEL             = 125,
        dLISP              = 210,
        dUnknown           = 255
    };

  protected:
    Ipv6RoutingTable *_rt; // TODO introduce IIPv6RoutingTable
    Ipv6Address _destPrefix;
    short _prefixLength;
    SourceType _sourceType;
    NetworkInterface *_interfacePtr;
    Ipv6Address _nextHop; // unspecified means "direct"
    simtime_t _expiryTime; // if route is an advertised prefix: prefix lifetime
    int _metric;
    unsigned int _adminDist;
    cObject *_source; /// Object identifying the source
    cObject *_protocolData; /// Routing Protocol specific data

  protected:
    void changed(int fieldCode);

  public:
    /**
     * Constructor. The destination prefix and the route source is passed
     * to the constructor and cannot be changed afterwards.
     */
    Ipv6Route(Ipv6Address destPrefix, int prefixLength, SourceType sourceType)
    {
        _rt = nullptr;
        _destPrefix = destPrefix;
        _prefixLength = prefixLength;
        _sourceType = sourceType;
        _interfacePtr = nullptr;
        _expiryTime = 0;
        _metric = 0;
        _adminDist = dUnknown;
        _source = nullptr;
        _protocolData = nullptr;
    }

    virtual ~Ipv6Route() { delete _protocolData; }

    virtual std::string str() const override;
    virtual std::string detailedInfo() const;

    /** To be called by the routing table when this route is added or removed from it */
    virtual void setRoutingTable(Ipv6RoutingTable *rt) { _rt = rt; }
    Ipv6RoutingTable *getRoutingTable() const { return _rt; }

    void setNextHop(const Ipv6Address& nextHop) { if (_nextHop != nextHop) { _nextHop = nextHop; changed(F_NEXTHOP); } }
    void setExpiryTime(simtime_t expiryTime) { if (expiryTime != _expiryTime) { _expiryTime = expiryTime; changed(F_EXPIRYTIME); } }
    void setMetric(int metric) override { if (_metric != metric) { _metric = metric; changed(F_METRIC); } }
    void setAdminDist(unsigned int adminDist) override { if (_adminDist != adminDist) { _adminDist = adminDist; changed(F_ADMINDIST); } }

    const Ipv6Address& getDestPrefix() const { return _destPrefix; }
    virtual int getPrefixLength() const override { return _prefixLength; }
    virtual SourceType getSourceType() const override { return _sourceType; }
    const Ipv6Address& getNextHop() const { return _nextHop; }
    simtime_t getExpiryTime() const { return _expiryTime; }
    virtual int getMetric() const override { return _metric; }
    unsigned int getAdminDist() const { return _adminDist; }
    virtual IRoutingTable *getRoutingTableAsGeneric() const override;

    virtual void setDestination(const L3Address& dest) override { if (_destPrefix != dest.toIpv6()) { _destPrefix = dest.toIpv6(); changed(F_DESTINATION); } }
    virtual void setPrefixLength(int prefixLength) override { if (_prefixLength != prefixLength) { _prefixLength = prefixLength; changed(F_PREFIX_LENGTH); } }
    virtual void setNextHop(const L3Address& nextHop) override { if (_nextHop != nextHop.toIpv6()) { _nextHop = nextHop.toIpv6(); changed(F_NEXTHOP); } }
    virtual void setSource(cObject *source) override { if (_source != source) { _source = source; changed(F_SOURCE); } }
    virtual void setSourceType(SourceType type) override { if (_sourceType != type) { _sourceType = type; changed(F_TYPE); } }
    const char *getSourceTypeAbbreviation() const;
    virtual L3Address getDestinationAsGeneric() const override { return getDestPrefix(); } // TODO rename Ipv6 method
    virtual L3Address getNextHopAsGeneric() const override { return getNextHop(); }
    virtual NetworkInterface *getInterface() const override { return _interfacePtr; }
    virtual void setInterface(NetworkInterface *ie) override { if (_interfacePtr != ie) { _interfacePtr = ie; changed(F_IFACE); } }
    virtual cObject *getSource() const override { return _source; }
    virtual cObject *getProtocolData() const override { return _protocolData; }
    virtual void setProtocolData(cObject *protocolData) override { _protocolData = protocolData; }
};

} // namespace inet

#endif

