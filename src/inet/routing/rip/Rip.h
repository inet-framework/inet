//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_RIP_H
#define __INET_RIP_H

#include "inet/common/ModuleRefByPar.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/contract/IRoute.h"
#include "inet/networklayer/contract/IRoutingTable.h"
#include "inet/routing/base/RoutingProtocolBase.h"
#include "inet/routing/rip/RipRouteData.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"

namespace inet {

#define RIP_INFINITE_METRIC    16

/**
 * Enumerated parameter to control how the Rip module
 * advertises the routes to its neighbors.
 */
enum RipMode {
    NO_RIP, // no updates are sent, and the attached network is not advertised
    PASSIVE, // no updates are sent, but the attached network is advertised
    NO_SPLIT_HORIZON, // every route is sent to the neighbor
    SPLIT_HORIZON, // do not send routes to the neighbor it was learnt from
    SPLIT_HORIZON_POISON_REVERSE // send the route to the neighbor it was learnt from with infinite metric (16)
};

/**
 * Holds the RIP configuration of the interfaces.
 * We could store this data in the NetworkInterface* itself,
 * but it contains only 5 holes for protocol data, and they
 * are used by network layer protocols only. Therefore
 * Rip manages its own table of these entries.
 */
struct RipNetworkInterface
{
    const NetworkInterface *ie = nullptr; // the associated interface entry
    int metric = 0; // metric of this interface
    RipMode mode = NO_RIP; // RIP mode of this interface

    RipNetworkInterface(const NetworkInterface *ie)
        : ie(ie), metric(1), mode(NO_RIP)
    {
        ASSERT(!ie->isLoopback());
        ASSERT(ie->isMulticast());
    }
};

/**
 * Implementation of the Routing Information Protocol.
 *
 * This module supports RIPv2 (RFC 2453) and RIPng (RFC 2080).
 *
 * RIP is a distance vector routing protocol. Each RIP router periodically sends its
 * whole routing table to neighbor routers, and updates its own routing table
 * according to the received information. If a route changed the router might send a
 * notification to its neighbors immediately (or rather with a small delay) which contains
 * only the changed routes (triggered updates).
 *
 * TODO
 * 1. Initially the router knows only the routes to the directly connected networks, and
 *    the routes that were manually configured. As it receives route updates from the
 *    neighbors it learns routes to remote networks.
 *    It should be possible to cooperate with other routing protocols that work in the same AS
 *    (e.g. OSPF) or with exterior protocols that connects this AS to other ASs (e.g. BGP).
 *    This requires some configurable criteria which routes of the routing table should be advertise
 *    by the RIP router, e.g. 'advertise the default route added by BGP with RIP metric 1'.
 *
 * 2. There is no merging of subnet routes. RFC 2453 3.7 suggests that subnetted network routes should
 *    not be advertised outside the subnetted network.
 */
class INET_API Rip : public RoutingProtocolBase, protected cListener
{
    enum Mode { RIPv2, RIPng };
    typedef std::vector<RipNetworkInterface> InterfaceVector;
    typedef std::vector<RipRoute *> RouteVector;
    // environment
    cModule *host = nullptr; // the host module that owns this module
    ModuleRefByPar<IInterfaceTable> ift;
    ModuleRefByPar<IRoutingTable> rt;
    const IL3AddressType *addressType = nullptr; // address type of the routing table
    // state
    InterfaceVector ripInterfaces; // interfaces on which RIP is used
    RouteVector ripRoutingTable; // all advertised routes (imported or learned)
    UdpSocket socket; // bound to the RIP port (see udpPort parameter)
    cMessage *updateTimer = nullptr; // for sending unsolicited Response messages in every ~30 seconds.
    cMessage *triggeredUpdateTimer = nullptr; // scheduled when there are pending changes
    cMessage *startupTimer = nullptr; // timer for delayed startup
    cMessage *shutdownTimer = nullptr; // scheduled at shutdown
    // parameters
    Mode mode = static_cast<Mode>(-1);
    int ripUdpPort = -1; // UDP port RIP routers (usually 520)
    simtime_t updateInterval; // time between regular updates
    simtime_t routeExpiryTime; // learned routes becomes invalid if no update received in this period of time
    simtime_t routePurgeTime; // invalid routes are deleted after this period of time is elapsed
    simtime_t holdDownTime;
    simtime_t shutdownTime; // time of shutdown processing
    bool triggeredUpdate = false;

    // signals
    static simsignal_t sentRequestSignal;
    static simsignal_t sentUpdateSignal;
    static simsignal_t rcvdResponseSignal;
    static simsignal_t badResponseSignal;
    static simsignal_t numRoutesSignal;

  public:
    Rip();
    ~Rip();

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;

    // lifecycle
    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

    virtual void startRIPRouting();
    virtual void stopRIPRouting();

    virtual RipRoute *importRoute(IRoute *route, RipRoute::RouteType type, int metric = 1, uint16_t routeTag = 0);
    virtual void sendRIPRequest(const RipNetworkInterface& ripInterface);

    virtual void processRequest(Packet *pk);
    virtual void processUpdate(bool triggered);
    virtual void sendRoutes(const L3Address& address, int port, const RipNetworkInterface& ripInterface, bool changedOnly);

    virtual void processResponse(Packet *pk);
    virtual bool isValidResponse(Packet *packet);
    virtual void updateRoute(RipRoute *route, const NetworkInterface *ie, const L3Address& nextHop, int metric, uint16_t routeTag, const L3Address& from);
    virtual void addRoute(const L3Address& dest, int prefixLength, const NetworkInterface *ie, const L3Address& nextHop, int metric, uint16_t routeTag, const L3Address& from);

    virtual void checkExpiredRoutes();
    virtual void invalidateRoute(RipRoute *route);
    virtual RouteVector::iterator purgeRoute(RipRoute *route);

    virtual void triggerUpdate();

    virtual void sendPacket(Packet *packet, const L3Address& destAddr, int destPort, const NetworkInterface *destInterface);

  private:
    RipNetworkInterface *findRipInterfaceById(int interfaceId);

    RipRoute *findRipRoute(const L3Address& destAddress, int prefixLength);
    RipRoute *findRipRoute(const L3Address& destination, int prefixLength, RipRoute::RouteType type);
    RipRoute *findRipRoute(const IRoute *route);
    RipRoute *findRipRoute(const NetworkInterface *ie, RipRoute::RouteType type);

    void addRipInterface(const NetworkInterface *ie, cXMLElement *config);
    void deleteRipInterface(const NetworkInterface *ie);
    int getInterfaceMetric(NetworkInterface *ie);

    IRoute *createRoute(const L3Address& dest, int prefixLength, const NetworkInterface *ie, const L3Address& nextHop, int metric);

    bool isLoopbackInterfaceRoute(const IRoute *route) {
        NetworkInterface *ie = dynamic_cast<NetworkInterface *>(route->getSource());
        return ie && ie->isLoopback();
    }

    bool isLocalInterfaceRoute(const IRoute *route) {
        NetworkInterface *ie = dynamic_cast<NetworkInterface *>(route->getSource());
        return ie && !ie->isLoopback();
    }

    bool isDefaultRoute(const IRoute *route) { return route->getPrefixLength() == 0; }
};

} // namespace inet

#endif

