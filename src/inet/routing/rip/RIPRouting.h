//
// Copyright (C) 2013 Opensim Ltd.
// Author: Tamas Borbely
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

#ifndef __INET_RIPROUTING_H
#define __INET_RIPROUTING_H

#include "inet/common/INETDefs.h"
#include "inet/networklayer/common/IRoute.h"
#include "inet/networklayer/common/IRoutingTable.h"
#include "inet/networklayer/common/IInterfaceTable.h"
#include "inet/common/lifecycle/ILifecycle.h"
#include "inet/transportlayer/contract/udp/UDPSocket.h"

namespace inet {

#define RIP_INFINITE_METRIC    16

struct RIPRoute : public cObject
{
    enum RouteType {
        RIP_ROUTE_RTE,    // route learned from a RIPEntry
        RIP_ROUTE_STATIC,    // static route
        RIP_ROUTE_DEFAULT,    // default route
        RIP_ROUTE_REDISTRIBUTE,    // route imported from another routing protocol
        RIP_ROUTE_INTERFACE    // route belongs to a local interface
    };

  private:
    RouteType type;    // the type of the route
    IRoute *route;    // the route in the host routing table that is associated with this route, may be NULL if deleted
    L3Address dest;    // destination of the route
    int prefixLength;    // prefix length of the destination
    L3Address nextHop;    // next hop of the route
    InterfaceEntry *ie;    // outgoing interface of the route
    L3Address from;    // only for RTE routes
    int metric;    // the metric of this route, or infinite (16) if invalid
    uint16 tag;    // route tag, only for REDISTRIBUTE routes
    bool changed;    // true if the route has changed since the update
    simtime_t lastUpdateTime;    // time of the last change, only for RTE routes

  public:
    RIPRoute(IRoute *route, RouteType type, int metric, uint16 tag);
    virtual std::string info() const;

    RouteType getType() const { return type; }
    IRoute *getRoute() const { return route; }
    L3Address getDestination() const { return dest; }
    int getPrefixLength() const { return prefixLength; }
    L3Address getNextHop() const { return nextHop; }
    InterfaceEntry *getInterface() const { return ie; }
    L3Address getFrom() const { return from; }
    int getMetric() const { return metric; }
    uint16 getRouteTag() const { return tag; }
    bool isChanged() const { return changed; }
    simtime_t getLastUpdateTime() const { return lastUpdateTime; }
    void setType(RouteType type) { this->type = type; }
    void setRoute(IRoute *route) { this->route = route; }
    void setDestination(const L3Address& dest) { this->dest = dest; }
    void setPrefixLength(int prefixLength) { this->prefixLength = prefixLength; }
    void setNextHop(const L3Address& nextHop) { this->nextHop = nextHop; if (route && type == RIP_ROUTE_RTE) route->setNextHop(nextHop); }
    void setInterface(InterfaceEntry *ie) { this->ie = ie; if (route && type == RIP_ROUTE_RTE) route->setInterface(ie); }
    void setMetric(int metric) { this->metric = metric; if (route && type == RIP_ROUTE_RTE) route->setMetric(metric); }
    void setRouteTag(uint16 routeTag) { this->tag = routeTag; }
    void setFrom(const L3Address& from) { this->from = from; }
    void setChanged(bool changed) { this->changed = changed; }
    void setLastUpdateTime(simtime_t time) { lastUpdateTime = time; }
};

/**
 * Enumerated parameter to control how the RIPRouting module
 * advertises the routes to its neighbors.
 */
enum RIPMode {
    NO_RIP,    // no RIP messages sent
    NO_SPLIT_HORIZON,    // every route is sent to the neighbor
    SPLIT_HORIZON,    // do not send routes to the neighbor it was learnt from
    SPLIT_HORIZON_POISONED_REVERSE    // send the route to the neighbor it was learnt from with infinite metric (16)
};

/**
 * Holds the RIP configuration of the interfaces.
 * We could store this data in the InterfaceEntry* itself,
 * but it contains only 5 holes for protocol data, and they
 * are used by network layer protocols only. Therefore
 * RIPRouting manages its own table of these entries.
 */
struct RIPInterfaceEntry
{
    const InterfaceEntry *ie;    // the associated interface entry
    int metric;    // metric of this interface
    RIPMode mode;    // RIP mode of this interface

    RIPInterfaceEntry(const InterfaceEntry *ie);
    void configure(cXMLElement *config);
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
class INET_API RIPRouting : public cSimpleModule, protected cListener, public ILifecycle
{
    enum Mode { RIPv2, RIPng };
    typedef std::vector<RIPInterfaceEntry> InterfaceVector;
    typedef std::vector<RIPRoute *> RouteVector;
    // environment
    cModule *host;    // the host module that owns this module
    IInterfaceTable *ift;    // interface table of the host
    IRoutingTable *rt;    // routing table from which routes are imported and to which learned routes are added
    IL3AddressType *addressType;    // address type of the routing table
    // state
    InterfaceVector ripInterfaces;    // interfaces on which RIP is used
    RouteVector ripRoutes;    // all advertised routes (imported or learned)
    UDPSocket socket;    // bound to the RIP port (see udpPort parameter)
    cMessage *updateTimer;    // for sending unsolicited Response messages in every ~30 seconds.
    cMessage *triggeredUpdateTimer;    // scheduled when there are pending changes
    cMessage *startupTimer;    // timer for delayed startup
    cMessage *shutdownTimer;    // scheduled at shutdown
    // parameters
    Mode mode;
    int ripUdpPort;    // UDP port RIP routers (usually 520)
    simtime_t updateInterval;    // time between regular updates
    simtime_t routeExpiryTime;    // learned routes becomes invalid if no update received in this period of time
    simtime_t routePurgeTime;    // invalid routes are deleted after this period of time is elapsed
    simtime_t shutdownTime;    // time of shutdown processing
    bool isOperational;

    // signals
    static simsignal_t sentRequestSignal;
    static simsignal_t sentUpdateSignal;
    static simsignal_t rcvdResponseSignal;
    static simsignal_t badResponseSignal;
    static simsignal_t numRoutesSignal;

  public:
    RIPRouting();
    ~RIPRouting();

  private:
    RIPInterfaceEntry *findInterfaceById(int interfaceId);
    RIPRoute *findRoute(const L3Address& destAddress, int prefixLength);
    RIPRoute *findRoute(const L3Address& destination, int prefixLength, RIPRoute::RouteType type);
    RIPRoute *findRoute(const IRoute *route);
    RIPRoute *findRoute(const InterfaceEntry *ie, RIPRoute::RouteType type);
    void addInterface(const InterfaceEntry *ie, cXMLElement *config);
    void deleteInterface(const InterfaceEntry *ie);
    void invalidateRoutes(const InterfaceEntry *ie);
    IRoute *addRoute(const L3Address& dest, int prefixLength, const InterfaceEntry *ie, const L3Address& nextHop, int metric);
    void deleteRoute(IRoute *route);
    bool isLoopbackInterfaceRoute(const IRoute *route);
    bool isLocalInterfaceRoute(const IRoute *route);
    bool isDefaultRoute(const IRoute *route) { return route->getPrefixLength() == 0; }
    std::string getHostName() { return host->getFullName(); }
    int getInterfaceMetric(InterfaceEntry *ie);

  protected:
    virtual int numInitStages() const { return NUM_INIT_STAGES; }
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg);
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj);
    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback);

    virtual void startRIPRouting();
    virtual void stopRIPRouting();

    virtual void configureInterfaces(cXMLElement *config);
    virtual void configureInitialRoutes();
    virtual RIPRoute *importRoute(IRoute *route, RIPRoute::RouteType type, int metric = 1, uint16 routeTag = 0);
    virtual void sendRIPRequest(const RIPInterfaceEntry& ripInterface);

    virtual void processRequest(RIPPacket *packet);
    virtual void processUpdate(bool triggered);
    virtual void sendRoutes(const L3Address& address, int port, const RIPInterfaceEntry& ripInterface, bool changedOnly);

    virtual void processResponse(RIPPacket *packet);
    virtual bool isValidResponse(RIPPacket *packet);
    virtual void addRoute(const L3Address& dest, int prefixLength, const InterfaceEntry *ie, const L3Address& nextHop, int metric, uint16 routeTag, const L3Address& from);
    virtual void updateRoute(RIPRoute *route, const InterfaceEntry *ie, const L3Address& nextHop, int metric, uint16 routeTag, const L3Address& from);

    virtual void triggerUpdate();
    virtual RIPRoute *checkRouteIsExpired(RIPRoute *route);
    virtual void invalidateRoute(RIPRoute *route);
    virtual void purgeRoute(RIPRoute *route);

    virtual void sendPacket(RIPPacket *packet, const L3Address& destAddr, int destPort, const InterfaceEntry *destInterface);
};

} // namespace inet

#endif // ifndef __INET_RIPROUTING_H

