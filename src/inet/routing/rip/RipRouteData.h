//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_RIPROUTEDATA_H
#define __INET_RIPROUTEDATA_H

#include <set>

#include "inet/networklayer/common/L3Address.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/contract/IRoute.h"

namespace inet {

class INET_API RipRoute : public cObject
{
  public:
    enum RouteType {
        RIP_ROUTE_RTE,          // route learned from a RipEntry
        RIP_ROUTE_STATIC,       // static route
        RIP_ROUTE_DEFAULT,      // default route
        RIP_ROUTE_REDISTRIBUTE, // route imported from another routing protocol
        RIP_ROUTE_INTERFACE     // route belongs to a local interface
    };

  protected:
    IRoute *route = nullptr; // the route in the host routing table that is associated with this route, may be nullptr if deleted
    RouteType type = static_cast<RouteType>(-1); // the type of the route

    L3Address dest; // destination of the route
    L3Address nextHop; // next hop of the route
    int prefixLength = 0; // prefix length of the destination
    int metric = 0; // the metric of this route, or infinite (16) if invalid
    NetworkInterface *ie = nullptr; // outgoing interface of the route

    L3Address from; // from which router did we get the route (only for RTE routes)
    simtime_t lastUpdateTime = 0; // time of the last change (only for RTE routes)
    bool changed = false; // true if the route has changed since the update
    uint16_t tag = 0; // route tag (only for REDISTRIBUTE routes)
    simtime_t lastInvalid = 0; // time of the last invalidation

  public:
    RipRoute(IRoute *route, RouteType type, int metric, uint16_t routeTag)
    {
        this->route = route;
        this->type = type;

        dest = route->getDestinationAsGeneric();
        nextHop = route->getNextHopAsGeneric();
        prefixLength = route->getPrefixLength();
        this->metric = metric;
        ie = route->getInterface();

        this->lastUpdateTime = 0;
        this->changed = false;
        this->tag = routeTag;
        this->lastInvalid = 0;
    }

    virtual std::string str() const override;

    IRoute *getRoute() const { return route; }
    RouteType getType() const { return type; }
    L3Address getDestination() const { return dest; }
    L3Address getNextHop() const { return nextHop; }
    int getPrefixLength() const { return prefixLength; }
    int getMetric() const { return metric; }
    NetworkInterface *getInterface() const { return ie; }
    L3Address getFrom() const { return from; }
    simtime_t getLastUpdateTime() const { return lastUpdateTime; }
    bool isChanged() const { return changed; }
    uint16_t getRouteTag() const { return tag; }
    simtime_t getLastInvalidationTime() const { return lastInvalid; }

    void setRoute(IRoute *route) { this->route = route; }
    void setType(RouteType type) { this->type = type; }
    void setDestination(const L3Address& dest) { this->dest = dest; }
    void setNextHop(const L3Address& nextHop) { this->nextHop = nextHop; if (route && type == RIP_ROUTE_RTE) route->setNextHop(nextHop); }
    void setPrefixLength(int prefixLength) { this->prefixLength = prefixLength; }
    void setMetric(int metric) { this->metric = metric; if (route && type == RIP_ROUTE_RTE) route->setMetric(metric); }
    void setInterface(NetworkInterface *ie) { this->ie = ie; if (route && type == RIP_ROUTE_RTE) route->setInterface(ie); }
    void setFrom(const L3Address& from) { this->from = from; }
    void setLastUpdateTime(simtime_t time) { lastUpdateTime = time; }
    void setChanged(bool changed) { this->changed = changed; }
    void setRouteTag(uint16_t routeTag) { this->tag = routeTag; }
    void setLastInvalidationTime(simtime_t time) { this->lastInvalid = time; }
};

} // namespace inet

#endif

