//
// Copyright (C) 2008 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IPV4ROUTE_H
#define __INET_IPV4ROUTE_H

#include "inet/networklayer/contract/IRoute.h"
#include "inet/networklayer/contract/ipv4/Ipv4Address.h"

namespace inet {

class NetworkInterface;
class IIpv4RoutingTable;

/**
 * Ipv4 unicast route in IIpv4RoutingTable.
 *
 * @see IIpv4RoutingTable, Ipv4RoutingTable
 */
class INET_API Ipv4Route : public cObject, public IRoute
{
  private:
    IIpv4RoutingTable *rt; ///< the routing table in which this route is inserted, or nullptr
    Ipv4Address dest; ///< Destination
    Ipv4Address netmask; ///< Route mask
    Ipv4Address gateway; ///< Next hop
    NetworkInterface *interfacePtr; ///< interface
    SourceType sourceType; ///< manual, routing prot, etc.
    unsigned int adminDist; ///< Cisco like administrative distance
    int metric; ///< Metric ("cost" to reach the destination)
    cObject *source; ///< Object identifying the source
    cObject *protocolData; ///< Routing Protocol specific data

  private:
    // copying not supported: following are private and also left undefined
    Ipv4Route(const Ipv4Route& obj);
    Ipv4Route& operator=(const Ipv4Route& obj);

  protected:
    void changed(int fieldCode);

  public:
    Ipv4Route() : rt(nullptr), interfacePtr(nullptr), sourceType(MANUAL), adminDist(dUnknown),
        metric(0), source(nullptr), protocolData(nullptr) {}
    virtual ~Ipv4Route();
    virtual std::string str() const override;
    virtual std::string detailedInfo() const;

    bool operator==(const Ipv4Route& route) const { return equals(route); }
    bool operator!=(const Ipv4Route& route) const { return !equals(route); }
    bool equals(const Ipv4Route& route) const;

    /** To be called by the routing table when this route is added or removed from it */
    virtual void setRoutingTable(IIpv4RoutingTable *rt) { this->rt = rt; }
    IIpv4RoutingTable *getRoutingTable() const { return rt; }

    /** test validity of route entry, e.g. check expiry */
    virtual bool isValid() const { return true; }

    virtual void setDestination(Ipv4Address _dest) { if (dest != _dest) { dest = _dest; changed(F_DESTINATION); } }
    virtual void setNetmask(Ipv4Address _netmask) { if (netmask != _netmask) { netmask = _netmask; changed(F_PREFIX_LENGTH); } }
    virtual void setGateway(Ipv4Address _gateway) { if (gateway != _gateway) { gateway = _gateway; changed(F_NEXTHOP); } }
    virtual void setInterface(NetworkInterface *_interfacePtr) override { if (interfacePtr != _interfacePtr) { interfacePtr = _interfacePtr; changed(F_IFACE); } }
    virtual void setSourceType(SourceType _source) override { if (sourceType != _source) { sourceType = _source; changed(F_SOURCE); } }
    const char *getSourceTypeAbbreviation() const;
    virtual void setAdminDist(unsigned int _adminDist) override { if (adminDist != _adminDist) { adminDist = _adminDist; changed(F_ADMINDIST); } }
    virtual void setMetric(int _metric) override { if (metric != _metric) { metric = _metric; changed(F_METRIC); } }

    /** Destination address prefix to match */
    const Ipv4Address& getDestination() const { return dest; }

    /** Represents length of prefix to match */
    const Ipv4Address& getNetmask() const { return netmask; }

    /** Next hop address */
    const Ipv4Address& getGateway() const { return gateway; }

    /** Next hop interface */
    NetworkInterface *getInterface() const override { return interfacePtr; }

    /** Convenience method */
    const char *getInterfaceName() const;

    /** Source of route. MANUAL (read from file), from routing protocol, etc */
    SourceType getSourceType() const override { return sourceType; }

    /** Route source specific preference value */
    unsigned int getAdminDist() const { return adminDist; }

    /** "Cost" to reach the destination */
    int getMetric() const override { return metric; }

    void setSource(cObject *_source) override { if (source != _source) { source = _source; changed(F_SOURCE); } }
    cObject *getSource() const override { return source; }

    cObject *getProtocolData() const override { return protocolData; }
    void setProtocolData(cObject *protocolData) override { this->protocolData = protocolData; }

    virtual IRoutingTable *getRoutingTableAsGeneric() const override;
    virtual void setDestination(const L3Address& dest) override { setDestination(dest.toIpv4()); }
    virtual void setPrefixLength(int len) override { setNetmask(Ipv4Address::makeNetmask(len)); }
    virtual void setNextHop(const L3Address& nextHop) override { setGateway(nextHop.toIpv4()); } // TODO rename Ipv4 method

    virtual L3Address getDestinationAsGeneric() const override { return getDestination(); }
    virtual int getPrefixLength() const override { return getNetmask().getNetmaskLength(); }
    virtual L3Address getNextHopAsGeneric() const override { return getGateway(); } // TODO rename Ipv4 method
};

/**
 * Ipv4 multicast route in IIpv4RoutingTable.
 * Multicast routing protocols may extend this class to store protocol
 * specific fields.
 *
 * Multicast datagrams are forwarded along the edges of a multicast tree.
 * The tree might depend on the multicast group and the source (origin) of
 * the multicast datagram.
 *
 * The forwarding algorithm chooses the route according the to origin
 * (source address) and multicast group (destination address) of the received
 * datagram. The route might specify a prefix of the origin and a multicast
 * group to be matched.
 *
 * Then the forwarding algorithm copies the datagrams arrived on the parent
 * (upstream) interface to the child interfaces (downstream). If there are no
 * downstream routers on a child interface (i.e. it is a leaf in the multicast
 * routing tree), then the datagram is forwarded only if there are listeners
 * of the multicast group on that link (TRPB routing).
 *
 * @see IIpv4RoutingTable, Ipv4RoutingTable
 */
class INET_API Ipv4MulticastRoute : public cObject, public IMulticastRoute
{
  private:
    IIpv4RoutingTable *rt; ///< the routing table in which this route is inserted, or nullptr
    Ipv4Address origin; ///< Source network
    Ipv4Address originNetmask; ///< Source network mask
    Ipv4Address group; ///< Multicast group, if unspecified then matches any
    InInterface *inInterface; ///< In interface (upstream)
    OutInterfaceVector outInterfaces; ///< Out interfaces (downstream)
    SourceType sourceType; ///< manual, routing prot, etc.
    cObject *source; ///< Object identifying the source
    int metric; ///< Metric ("cost" to reach the source)

  public:
    // field codes for changed()
    enum { F_ORIGIN, F_ORIGINMASK, F_MULTICASTGROUP, F_IN, F_OUT, F_SOURCE, F_METRIC, F_LAST };

  protected:
    void changed(int fieldCode);

  private:
    // copying not supported: following are private and also left undefined
    Ipv4MulticastRoute(const Ipv4MulticastRoute& obj);
    Ipv4MulticastRoute& operator=(const Ipv4MulticastRoute& obj);

  public:
    Ipv4MulticastRoute() : rt(nullptr), inInterface(nullptr), sourceType(MANUAL), source(nullptr), metric(0) {}
    virtual ~Ipv4MulticastRoute();
    virtual std::string str() const override;
    virtual std::string detailedInfo() const;

    /** To be called by the routing table when this route is added or removed from it */
    virtual void setRoutingTable(IIpv4RoutingTable *rt) { this->rt = rt; }
    IIpv4RoutingTable *getRoutingTable() const { return rt; }

    /** test validity of route entry, e.g. check expiry */
    virtual bool isValid() const { return true; }

    virtual bool matches(const Ipv4Address& origin, const Ipv4Address& group) const
    {
        return (this->group.isUnspecified() || this->group == group) &&
               Ipv4Address::maskedAddrAreEqual(origin, this->origin, this->originNetmask);
    }

    virtual void setOrigin(Ipv4Address _origin) { if (origin != _origin) { origin = _origin; changed(F_ORIGIN); } }
    virtual void setOriginNetmask(Ipv4Address _netmask) { if (originNetmask != _netmask) { originNetmask = _netmask; changed(F_ORIGINMASK); } }
    virtual void setMulticastGroup(Ipv4Address _group) { if (group != _group) { group = _group; changed(F_MULTICASTGROUP); } }
    virtual void setInInterface(InInterface *_inInterface) override;
    virtual void clearOutInterfaces() override;
    virtual void addOutInterface(OutInterface *outInterface) override;
    virtual bool removeOutInterface(const NetworkInterface *ie) override;
    virtual void removeOutInterface(unsigned int i) override;
    virtual void setSourceType(SourceType _source) override { if (sourceType != _source) { sourceType = _source; changed(F_SOURCE); } }
    virtual void setMetric(int _metric) override { if (metric != _metric) { metric = _metric; changed(F_METRIC); } }

    /** Source address prefix to match */
    Ipv4Address getOrigin() const { return origin; }

    /** Represents length of prefix to match */
    Ipv4Address getOriginNetmask() const { return originNetmask; }

    /** Multicast group address */
    Ipv4Address getMulticastGroup() const { return group; }

    /** In interface */
    InInterface *getInInterface() const { return inInterface; }

    /** Number of out interfaces */
    unsigned int getNumOutInterfaces() const { return outInterfaces.size(); }

    /** The kth out interface */
    OutInterface *getOutInterface(unsigned int k) const { return outInterfaces.at(k); }

    /** Source of route. MANUAL (read from file), from routing protocol, etc */
    SourceType getSourceType() const override { return sourceType; }

    /** "Cost" to reach the destination */
    int getMetric() const override { return metric; }

    void setSource(cObject *_source) override { source = _source; }
    cObject *getSource() const override { return source; }

    virtual IRoutingTable *getRoutingTableAsGeneric() const override;
    virtual void setEnabled(bool enabled) override { /*TODO setEnabled(enabled);*/ }
    virtual void setOrigin(const L3Address& origin) override { setOrigin(origin.toIpv4()); }
    virtual void setPrefixLength(int len) override { setOriginNetmask(Ipv4Address::makeNetmask(len)); } // TODO inconsistent naming
    virtual void setMulticastGroup(const L3Address& group) override { setMulticastGroup(group.toIpv4()); }

    virtual bool isEnabled() const override { return true; /*TODO isEnabled();*/ }
    virtual bool isExpired() const override { return !isValid(); } // TODO rename Ipv4 method
    virtual L3Address getOriginAsGeneric() const override { return getOrigin(); }
    virtual int getPrefixLength() const override { return getOriginNetmask().getNetmaskLength(); } // TODO inconsistent naming
    virtual L3Address getMulticastGroupAsGeneric() const override { return getMulticastGroup(); }
};

} // namespace inet

#endif

