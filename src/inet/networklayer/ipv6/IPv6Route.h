//
// Copyright (C) 2005 Andras Varga
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

#ifndef __INET_IPV6ROUTE_H
#define __INET_IPV6ROUTE_H

#include <vector>

#include "inet/common/INETDefs.h"

#include "inet/networklayer/common/IRoute.h"
#include "inet/networklayer/contract/ipv6/IPv6Address.h"

namespace inet {

class InterfaceEntry;
class IPv6RoutingTable;

/**
 * Represents a route in the route table. Routes with sourceType=ROUTER_ADVERTISEMENT represent
 * on-link prefixes advertised by routers.
 */
class INET_API IPv6Route : public cObject, public IRoute
{
  public:
    /** Cisco like administrative distances (includes IPv4 protocols)*/
    enum RouteAdminDist {
        dDirectlyConnected = 0,
        dStatic = 1,
        dEIGRPSummary = 5,
        dBGPExternal = 20,
        dEIGRPInternal = 90,
        dIGRP = 100,
        dOSPF = 110,
        dISIS = 115,
        dRIP = 120,
        dEGP = 140,
        dODR = 160,
        dEIGRPExternal = 170,
        dBGPInternal = 200,
        dDHCPlearned = 254,
        dUnknown = 255
    };

  protected:
    IPv6RoutingTable *_rt;    // TODO introduce IIPv6RoutingTable
    IPv6Address _destPrefix;
    short _prefixLength;
    SourceType _sourceType;
    InterfaceEntry *_interfacePtr;
    IPv6Address _nextHop;    // unspecified means "direct"
    simtime_t _expiryTime;    // if route is an advertised prefix: prefix lifetime
    int _metric;
    unsigned int _adminDist;
    cObject *_source;    /// Object identifying the source
    cObject *_protocolData;    /// Routing Protocol specific data

  protected:
    void changed(int fieldCode);

  public:
    /**
     * Constructor. The destination prefix and the route source is passed
     * to the constructor and cannot be changed afterwards.
     */
    IPv6Route(IPv6Address destPrefix, int prefixLength, SourceType sourceType)
    {
        _rt = NULL;
        _destPrefix = destPrefix;
        _prefixLength = prefixLength;
        _sourceType = sourceType;
        _interfacePtr = NULL;
        _expiryTime = 0;
        _metric = 0;
        _adminDist = dUnknown;
        _source = NULL;
        _protocolData = NULL;
    }

    virtual ~IPv6Route() { delete _protocolData; }

    virtual std::string info() const;
    virtual std::string detailedInfo() const;

    /** To be called by the routing table when this route is added or removed from it */
    virtual void setRoutingTable(IPv6RoutingTable *rt) { _rt = rt; }
    IPv6RoutingTable *getRoutingTable() const { return _rt; }

    void setNextHop(const IPv6Address& nextHop) { if (_nextHop != nextHop) { _nextHop = nextHop; changed(F_NEXTHOP); } }
    void setExpiryTime(simtime_t expiryTime) { if (expiryTime != _expiryTime) { _expiryTime = expiryTime; changed(F_EXPIRYTIME); } }
    void setMetric(int metric) { if (_metric != metric) { _metric = metric; changed(F_METRIC); } }
    void setAdminDist(unsigned int adminDist) { if (_adminDist != adminDist) { _adminDist = adminDist; changed(F_ADMINDIST); } }

    const IPv6Address& getDestPrefix() const { return _destPrefix; }
    virtual int getPrefixLength() const { return _prefixLength; }
    virtual SourceType getSourceType() const { return _sourceType; }
    const IPv6Address& getNextHop() const { return _nextHop; }
    simtime_t getExpiryTime() const { return _expiryTime; }
    virtual int getMetric() const { return _metric; }
    unsigned int getAdminDist() const { return _adminDist; }
    virtual IRoutingTable *getRoutingTableAsGeneric() const;

    virtual void setDestination(const L3Address& dest) { if (_destPrefix != dest.toIPv6()) { _destPrefix = dest.toIPv6(); changed(F_DESTINATION); } }
    virtual void setPrefixLength(int prefixLength) { if (_prefixLength != prefixLength) { _prefixLength = prefixLength; changed(F_PREFIX_LENGTH); } }
    virtual void setNextHop(const L3Address& nextHop) { if (_nextHop != nextHop.toIPv6()) { _nextHop = nextHop.toIPv6(); changed(F_NEXTHOP); } }
    virtual void setSource(cObject *source) { if (_source != source) { _source = source; changed(F_SOURCE); } }
    virtual void setSourceType(SourceType type) { if (_sourceType != type) { _sourceType = type; changed(F_TYPE); } }
    virtual L3Address getDestinationAsGeneric() const { return getDestPrefix(); }    //TODO rename IPv6 method
    virtual L3Address getNextHopAsGeneric() const { return getNextHop(); }
    virtual InterfaceEntry *getInterface() const { return _interfacePtr; }
    virtual void setInterface(InterfaceEntry *ie) { if (_interfacePtr != ie) { _interfacePtr = ie; changed(F_IFACE); } }
    virtual cObject *getSource() const { return _source; }
    virtual cObject *getProtocolData() const { return _protocolData; }
    virtual void setProtocolData(cObject *protocolData) { _protocolData = protocolData; }
};

} // namespace inet

#endif // ifndef __INET_IPV6ROUTE_H

