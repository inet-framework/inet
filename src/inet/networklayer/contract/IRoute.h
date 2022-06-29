//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IROUTE_H
#define __INET_IROUTE_H

#include "inet/networklayer/common/L3Address.h"
#include "inet/networklayer/common/NetworkInterface.h"

namespace inet {

class IRoutingTable;

/**
 * C++ interface for accessing unicast routing table entries of various protocols (IPv4, IPv6, etc)
 * in a uniform way.
 *
 * @see IRoutingTable, Ipv4Route, Ipv6Route
 */
class INET_API IRoute
{
  public:
    /** Specifies where the route comes from */
    enum SourceType {
        MANUAL, ///< manually added static route
        IFACENETMASK, ///< comes from an interface's netmask
        ROUTER_ADVERTISEMENT, ///< on-link prefix, from Router Advertisement
        OWN_ADV_PREFIX, ///< on routers: on-link prefix that the router **itself** advertises on the link
        ICMP_REDIRECT, ///< ICMP redirect message
        RIP, ///< managed by the given routing protocol
        OSPF, ///< managed by the given routing protocol
        BGP, ///< managed by the given routing protocol
        ZEBRA, ///< managed by the Quagga/Zebra based model
        MANET, ///< managed by manet, search exact address
        MANET2, ///< managed by manet, search approximate address
        DYMO, ///< managed by DYMO routing
        AODV, ///< managed by AODV routing
        EIGRP, LISP, BABEL, ODR, UNKNOWN, ISIS
    };

    /** Field codes for NB_ROUTE_CHANGED notifications */
    enum ChangeCodes {
        F_DESTINATION,
        F_PREFIX_LENGTH,
        F_NEXTHOP,
        F_IFACE,
        F_SOURCE,
        F_TYPE,
        F_ADMINDIST,
        F_METRIC,
        F_EXPIRYTIME,
        F_LAST
    };

    /** Cisco like administrative distances */
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

    // TODO maybe:
//    virtual std::string info() const;
//    virtual std::string detailedInfo() const;
//
//    bool operator==(const IRoute& route) const { return equals(route); }
//    bool operator!=(const IRoute& route) const { return !equals(route); }
//    bool equals(const IRoute& route) const;

    virtual ~IRoute() {}

    /** The routing table in which this route is inserted, or nullptr. */
    virtual IRoutingTable *getRoutingTableAsGeneric() const = 0;

    virtual void setDestination(const L3Address& dest) = 0;
    virtual void setPrefixLength(int l) = 0;
    virtual void setNextHop(const L3Address& nextHop) = 0;
    virtual void setInterface(NetworkInterface *ie) = 0;
    virtual void setSource(cObject *source) = 0;
    virtual void setSourceType(SourceType type) = 0;
    virtual void setMetric(int metric) = 0; // TODO double?
    virtual void setAdminDist(unsigned int adminDist) = 0;

    /** Destination address prefix to match */
    virtual L3Address getDestinationAsGeneric() const = 0;

    /** Represents length of prefix to match */
    virtual int getPrefixLength() const = 0;

    /** Next hop address */
    virtual L3Address getNextHopAsGeneric() const = 0;

    /** Next hop interface */
    virtual NetworkInterface *getInterface() const = 0;

    /** Source of route */
    virtual cObject *getSource() const = 0;

    /** Source type of the route */
    virtual SourceType getSourceType() const = 0;

    /** Cost to reach the destination */
    virtual int getMetric() const = 0;

    virtual cObject *getProtocolData() const = 0;
    virtual void setProtocolData(cObject *protocolData) = 0;

    static const char *sourceTypeName(SourceType sourceType);
};

// TODO move into info()?
inline std::ostream& operator<<(std::ostream& out, const IRoute *route)
{
    out << "destination = " << route->getDestinationAsGeneric();
    out << ", prefixLength = " << route->getPrefixLength();
    out << ", nextHop = " << route->getNextHopAsGeneric();
    out << ", metric = " << route->getMetric();
    if (auto ie = route->getInterface())
        out << ", interface = " << ie->getInterfaceName();
    return out;
};

/**
 * Generic multicast route in an IRoutingTable.
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
 * @see IRoutingTable, Ipv4MulticastRoute, IPv6MulticastRoute
 */
class INET_API IMulticastRoute
{
  public:
    /** Specifies where the route comes from */
    enum SourceType {
        MANUAL, ///< manually added static route
        DVMRP, ///< managed by DVMRP router
        PIM_DM, ///< managed by PIM-DM router
        PIM_SM, ///< managed by PIM-SM router
    };

    class INET_API InInterface {
      protected:
        NetworkInterface *ie;

      public:
        InInterface(NetworkInterface *ie) : ie(ie) { ASSERT(ie); }
        virtual ~InInterface() {}

        NetworkInterface *getInterface() const { return ie; }
    };

    class INET_API OutInterface {
      protected:
        const NetworkInterface *ie;
        bool _isLeaf; // for TRPB support

      public:
        OutInterface(const NetworkInterface *ie, bool isLeaf = false) : ie(ie), _isLeaf(isLeaf) { ASSERT(ie); }
        OutInterface(const OutInterface& other) : ie(other.ie), _isLeaf(other._isLeaf) {}
        virtual ~OutInterface() {}

        const NetworkInterface *getInterface() const { return ie; }
        bool isLeaf() const { return _isLeaf; }

        // to disable forwarding on this interface (e.g. pruned by PIM)
        virtual bool isEnabled() { return true; }
    };

    typedef std::vector<OutInterface *> OutInterfaceVector;

    // TODO maybe:
//    virtual std::string info() const;
//    virtual std::string detailedInfo() const;

    virtual ~IMulticastRoute() {}

    /** The routing table in which this route is inserted, or nullptr. */
    virtual IRoutingTable *getRoutingTableAsGeneric() const = 0;

    virtual void setEnabled(bool enabled) = 0;
    virtual void setOrigin(const L3Address& origin) = 0;
    virtual void setPrefixLength(int len) = 0;
    virtual void setMulticastGroup(const L3Address& group) = 0;
    virtual void setInInterface(InInterface *_inInterface) = 0;
    virtual void clearOutInterfaces() = 0;
    virtual void addOutInterface(OutInterface *outInterface) = 0;
    virtual bool removeOutInterface(const NetworkInterface *ie) = 0;
    virtual void removeOutInterface(unsigned int i) = 0;
    virtual void setSource(cObject *source) = 0;
    virtual void setSourceType(SourceType type) = 0;
    virtual void setMetric(int metric) = 0;

    /** Disabled entries are ignored by routing until the became enabled again. */
    virtual bool isEnabled() const = 0;

    /** Expired entries are ignored by routing, and may be periodically purged. */
    virtual bool isExpired() const = 0;

    /** Source address prefix to match */
    virtual L3Address getOriginAsGeneric() const = 0;

    /** Prefix length to match */
    virtual int getPrefixLength() const = 0;

    /** Multicast group address */
    virtual L3Address getMulticastGroupAsGeneric() const = 0;

    /** Source of route */
    virtual cObject *getSource() const = 0;

    /** Source type of the route */
    virtual SourceType getSourceType() const = 0;

    /** Cost to reach the destination */
    virtual int getMetric() const = 0;

    static const char *sourceTypeName(SourceType sourceType);
};

} // namespace inet

#endif

