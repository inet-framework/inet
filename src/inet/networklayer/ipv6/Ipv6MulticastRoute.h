//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IPV6MULTICASTROUTE_H
#define __INET_IPV6MULTICASTROUTE_H

#include <vector>

#include "inet/networklayer/contract/IRoute.h"
#include "inet/networklayer/contract/ipv6/Ipv6Address.h"

namespace inet {

class NetworkInterface;
class Ipv6RoutingTable;

/**
 * An IPv6 multicast route in the IPv6 multicast routing table (RIB/FIB).
 * Multicast routing protocols (e.g. PIM) may extend this class to store
 * protocol-specific fields. This is the IPv6 counterpart of Ipv4MulticastRoute.
 *
 * Multicast datagrams are forwarded along the edges of a multicast tree.
 * The tree might depend on the multicast group and the source (origin) of
 * the multicast datagram.
 *
 * The forwarding algorithm chooses the route according to the origin
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
 * @see Ipv6RoutingTable, Ipv4MulticastRoute
 */
class INET_API Ipv6MulticastRoute : public cObject, public IMulticastRoute
{
  private:
    Ipv6RoutingTable *rt = nullptr; ///< the routing table in which this route is inserted, or nullptr
    Ipv6Address origin; ///< Source network
    int prefixLength = 0; ///< Length of the origin prefix to match
    Ipv6Address group; ///< Multicast group, if unspecified then matches any
    InInterface *inInterface = nullptr; ///< In interface (upstream)
    OutInterfaceVector outInterfaces; ///< Out interfaces (downstream)
    SourceType sourceType = MANUAL; ///< manual, routing prot, etc.
    cObject *source = nullptr; ///< Object identifying the source
    int metric = 0; ///< Metric ("cost" to reach the source)

  public:
    // field codes for changed()
    enum { F_ORIGIN, F_ORIGINPREFIX, F_MULTICASTGROUP, F_IN, F_OUT, F_SOURCE, F_METRIC, F_LAST };

  protected:
    void changed(int fieldCode);

  private:
    // copying not supported: following are private and also left undefined
    Ipv6MulticastRoute& operator=(const Ipv6MulticastRoute& obj);

  public:
    Ipv6MulticastRoute() {}
    Ipv6MulticastRoute(const Ipv6MulticastRoute& other);
    virtual ~Ipv6MulticastRoute();
    virtual std::string str() const override;
    virtual std::string detailedInfo() const;

    /** To be called by the routing table when this route is added or removed from it */
    virtual void setRoutingTable(Ipv6RoutingTable *rt) { this->rt = rt; }
    Ipv6RoutingTable *getRoutingTable() const { return rt; }

    virtual bool matches(const Ipv6Address& origin, const Ipv6Address& group) const;

    virtual void setOrigin(const Ipv6Address& _origin) { if (origin != _origin) { origin = _origin; changed(F_ORIGIN); } }
    virtual void setPrefixLength(int _prefixLength) override { if (prefixLength != _prefixLength) { prefixLength = _prefixLength; changed(F_ORIGINPREFIX); } }
    virtual void setMulticastGroup(const Ipv6Address& _group) { if (group != _group) { group = _group; changed(F_MULTICASTGROUP); } }
    virtual void setInInterface(InInterface *_inInterface) override;
    virtual void clearOutInterfaces() override;
    virtual void addOutInterface(OutInterface *outInterface) override;
    virtual bool removeOutInterface(const NetworkInterface *ie) override;
    virtual void removeOutInterface(unsigned int i) override;
    virtual void setSourceType(SourceType _source) override { if (sourceType != _source) { sourceType = _source; changed(F_SOURCE); } }
    virtual void setMetric(int _metric) override { if (metric != _metric) { metric = _metric; changed(F_METRIC); } }

    /** Source address prefix to match */
    Ipv6Address getOrigin() const { return origin; }

    /** Multicast group address */
    Ipv6Address getMulticastGroup() const { return group; }

    /** In interface */
    InInterface *getInInterface() const { return inInterface; }

    /** Number of out interfaces */
    unsigned int getNumOutInterfaces() const { return outInterfaces.size(); }

    /** The kth out interface */
    OutInterface *getOutInterface(unsigned int k) const { return outInterfaces.at(k); }

    bool hasOutInterface(const NetworkInterface *networkInterface) const;

    /** Source of route. MANUAL (read from file), from routing protocol, etc */
    SourceType getSourceType() const override { return sourceType; }

    /** "Cost" to reach the destination */
    int getMetric() const override { return metric; }

    void setSource(cObject *_source) override { source = _source; }
    cObject *getSource() const override { return source; }

    virtual IRoutingTable *getRoutingTableAsGeneric() const override;
    virtual void setOrigin(const L3Address& _origin) override { setOrigin(_origin.toIpv6()); }
    virtual void setMulticastGroup(const L3Address& _group) override { setMulticastGroup(_group.toIpv6()); }

    virtual L3Address getOriginAsGeneric() const override { return getOrigin(); }
    virtual int getPrefixLength() const override { return prefixLength; }
    virtual L3Address getMulticastGroupAsGeneric() const override { return getMulticastGroup(); }
};

} // namespace inet

#endif
