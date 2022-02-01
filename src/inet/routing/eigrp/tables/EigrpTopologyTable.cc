//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
/**
 * @author Jan Zavrel (honza.zavrel96@gmail.com)
 * @author Jan Bloudicek (jbloudicek@gmail.com)
 * @author Vit Rek (rek@kn.vutbr.cz)
 * @author Vladimir Vesely (ivesely@fit.vutbr.cz)
 * @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3
 */

// Ipv6 ready - mozny problem s originator (address vs. routerID)!!!
#include "inet/routing/eigrp/tables/EigrpTopologyTable.h"

#include <algorithm>

#include "inet/networklayer/common/L3Address.h"
#include "inet/routing/eigrp/messages/EigrpMessage_m.h"
#include "inet/routing/eigrp/pdms/EigrpMetricHelper.h"
namespace inet {

template<typename IPAddress>
std::ostream& operator<<(std::ostream& os, const EigrpRouteSource<IPAddress>& source)
{
    EigrpRoute<IPAddress> *route = source.getRouteInfo();

    const char *state = route->isActive() ? "A" : "P";
//    const char *source = route.isInternal()() ? "internal" : "external";

    os << "" << state << "  ";
//    os << "ID:" << source.getSourceId() << "  ";
    os << route->getRouteAddress() << "/" << getNetmaskLength(route->getRouteMask());
    if (source.isSuccessor())
        os << "  is successor";
    os << "  FD:" << route->getFd();
    if (source.getNextHop().isUnspecified())
        os << "  via Connected ";
    else
        os << "  via " << source.getNextHop();
    os << " (" << source.getMetric() << "/" << source.getRd() << ")";
    os << ",  IF:" << source.getIfaceName() << "(" << source.getIfaceId() << ")";
//    os << "  originator:" << source.getOriginator();

    return os;
}

template<typename IPAddress>
std::ostream& operator<<(std::ostream& os, const EigrpRoute<IPAddress>& route)
{
    os << "ID: " << route.getRouteId() << "  ";
    os << route.getRouteAddress() << "/" << getNetmaskLength(route.getRouteMask());
    os << "  queryOrigin:" << route.getQueryOrigin();
    os << "  replyStatus:" << route.getReplyStatusSum();

    return os;
}

namespace eigrp {

#define EIGRP_TT_DEBUG

Define_Module(EigrpIpv4TopologyTable);

#ifndef DISABLE_EIGRP_IPV6
Define_Module(EigrpIpv6TopologyTable);
#endif /* DISABLE_EIGRP_IPV6 */

template<typename IPAddress>
void EigrpTopologyTable<IPAddress>::initialize(int stage)
{
    cSimpleModule::initialize(stage);
    if (stage == INITSTAGE_ROUTING_PROTOCOLS) {
        // TODO
        WATCH(routerID);
        WATCH_PTRVECTOR(routeVec);
        #ifdef EIGRP_TT_DEBUG
        WATCH_PTRVECTOR(routeInfoVec);
        #endif
    }
}

template<typename IPAddress>
void EigrpTopologyTable<IPAddress>::handleMessage(cMessage *msg)
{
    throw cRuntimeError("This module does not process messages");
}

template<typename IPAddress>
EigrpTopologyTable<IPAddress>::~EigrpTopologyTable()
{
    EigrpRouteSource<IPAddress> *rtSrc;
    EigrpRoute<IPAddress> *rt;
    unsigned i;

    for (i = 0; i < routeVec.size(); i++) {
        rtSrc = routeVec[i];
        routeVec[i] = nullptr;
        delete rtSrc;
    }
    routeVec.clear();

    for (i = 0; i < routeInfoVec.size(); i++) {
        rt = routeInfoVec[i];
        routeInfoVec[i] = nullptr;
        delete rt;
    }
    routeInfoVec.clear();
}

/**
 */
template<typename IPAddress>
EigrpRouteSource<IPAddress> *EigrpTopologyTable<IPAddress>::findRoute(const IPAddress& routeAddr, const IPAddress& routeMask, const IPAddress& nextHop)
{
    typename RouteVector::iterator it;
    EigrpRoute<IPAddress> *route;

    for (it = routeVec.begin(); it != routeVec.end(); it++) {
        route = (*it)->getRouteInfo();
        if (route->getRouteAddress() == routeAddr && route->getRouteMask() == routeMask &&
            (*it)->getNextHop() == nextHop && (*it)->isValid())
        {
            return *it;
        }
    }

    return nullptr;
}

template<typename IPAddress>
EigrpRouteSource<IPAddress> *EigrpTopologyTable<IPAddress>::findRoute(const IPAddress& routeAddr, const IPAddress& routeMask, int nextHopId)
{
    typename RouteVector::iterator it;
    EigrpRoute<IPAddress> *route;

    for (it = routeVec.begin(); it != routeVec.end(); it++) {
        route = (*it)->getRouteInfo();

        if (route->getRouteAddress() == routeAddr && route->getRouteMask() == routeMask &&
            (*it)->getNexthopId() == nextHopId && (*it)->isValid())
        {
            return *it;
        }
    }

    return nullptr;
}

template<typename IPAddress>
uint64_t EigrpTopologyTable<IPAddress>::findRouteDMin(EigrpRoute<IPAddress> *route)
{
    uint64_t dmin = eigrp::EigrpMetricHelper::METRIC_INF;
    uint64_t tempD;
    typename RouteVector::iterator it;
    int routeId = route->getRouteId();

    for (it = routeVec.begin(); it != routeVec.end(); it++) {
        if ((*it)->getRouteId() == routeId /* do not check FC here */ && (*it)->isValid()) {
            tempD = (*it)->getMetric();
            if (tempD < dmin) {
                dmin = tempD;
            }
        }
    }

    return dmin;
}

/**
 * Finds successor for the route with minimal metric, second key is minimal next hop IP.
 */
template<typename IPAddress>
EigrpRouteSource<IPAddress> *EigrpTopologyTable<IPAddress>::getBestSuccessor(EigrpRoute<IPAddress> *route)
{
    typename RouteVector::iterator it;
    int routeId = route->getRouteId();
    EigrpRouteSource<IPAddress> *tempSrc = nullptr;

    for (it = routeVec.begin(); it != routeVec.end(); it++) {
        if ((*it)->getRouteId() == routeId && (*it)->isSuccessor() && (*it)->isValid()) {
            if (tempSrc != nullptr) {
                if ((*it)->getMetric() < tempSrc->getMetric())
                    tempSrc = *it;
                else if ((*it)->getMetric() == tempSrc->getMetric() && (*it)->getNextHop() < tempSrc->getNextHop())
                    tempSrc = *it;
            }
            else
                tempSrc = *it;
        }
    }

    return tempSrc;
}

/*EigrpRouteSource<IPv4Address> *EigrpIpv4TopologyTable::getFirstSuccessor(const IPv4Address& address, const IPv4Address& mask)
{
    RouteInfoVector::iterator it;

    for (it = routeInfoVec.begin(); it != routeInfoVec.end(); it++)
    {
        if ((*it)->getRouteAddress() == address && (*it)->getRouteMask() == mask && (*it)->isValid())
        {
            return (*it)->getSuccessor();
        }
    }

    return nullptr;
}*/

/**
 * Finds successor on specified interface with metric equal to Dij (minimal distance)
 */
template<typename IPAddress>
EigrpRouteSource<IPAddress> *EigrpTopologyTable<IPAddress>::getBestSuccessorByIf(EigrpRoute<IPAddress> *route, int ifaceId)
{
    typename RouteVector::iterator it;
    int routeId = route->getRouteId();
    uint64_t dij = route->getDij();

    for (it = routeVec.begin(); it != routeVec.end(); it++) {
        if ((*it)->getRouteId() == routeId && (*it)->isSuccessor() && (*it)->getMetric() == dij &&
            (*it)->getIfaceId() == ifaceId && (*it)->isValid())
        {
            return *it;
        }
    }
    return nullptr;
}

/**
 * Finds feasible successor and minimal distance to the destination.
 *
 * @params resultDmin Return value with minimal distance of all FS.
 */
template<typename IPAddress>
bool EigrpTopologyTable<IPAddress>::hasFeasibleSuccessor(EigrpRoute<IPAddress> *route, uint64_t& resultDmin)
{
    typename RouteVector::iterator it;
    int routeId = route->getRouteId();
    bool hasFs = false;
    uint64_t tempD;

    resultDmin = eigrp::EigrpMetricHelper::METRIC_INF;
    EV_DEBUG << "EIGRP: Search feasible successor for route " << route->getRouteAddress();
    EV_DEBUG << ", FD is " << route->getFd() << endl;

    for (it = routeVec.begin(); it != routeVec.end(); it++) {
        if ((*it)->getRouteId() == routeId && (*it)->isValid()) {
            EV_DEBUG << "    Next hop " << (*it)->getNextHop();
            EV_DEBUG << " (" << (*it)->getMetric() << "/" << (*it)->getRd() << ") ";

            tempD = (*it)->getMetric();
            if (tempD < resultDmin) {
                hasFs = false; // FS must have minimal distance
                resultDmin = tempD;
            }

            if ((*it)->getRd() < route->getFd() && tempD == resultDmin) {
                EV_DEBUG << "satisfies FC" << endl;
                hasFs = true;
            }
            else
                EV_DEBUG << "not satisfies FC" << endl;
        }
    }
    if (hasFs)
        EV_DEBUG << "    FS found, dmin is " << resultDmin << endl;
    else
        EV_DEBUG << "    FS not found, dmin is " << resultDmin << endl;

    return hasFs;
}

template<typename IPAddress>
void EigrpTopologyTable<IPAddress>::addRoute(EigrpRouteSource<IPAddress> *source)
{
//    RouteInfoVector::reverse_iterator rit;
    typename RouteVector::iterator it = routeVec.end();
    int routeId = source->getRouteId();

    source->setSourceId(sourceIdCounter);
    sourceIdCounter++;

    // Find last route with routeId
    while (it != routeVec.begin()) {
        --it;
        if ((*it)->getRouteId() == routeId && (*it)->isValid()) {
            break;
        }
    }

    // Insert item after found item
    if (it == routeVec.end())
        routeVec.push_back(source);
    else
        routeVec.insert(it + 1, source);
}

/*
void EigrpIpv4TopologyTable::moveSuccessorAhead(EigrpRouteSource<IPv4Address> *source)
{
    RouteVector::iterator it, backit;
    EigrpRouteSource<IPv4Address> *tempSource = nullptr;

    if ((it = std::find(routeVec.begin(), routeVec.end(), source)) == routeVec.end())
    {
        return;
    }

    if (it == routeVec.begin())
        return;

    backit = it - 1;

    // Move successor before all nonsuccessors
    while (it != routeVec.begin())
    {
         if ((*it)->getRouteId() != (*backit)->getRouteId())
             break;

         if ((*backit)->isSuccessor() && (*backit)->getNextHop() <= (*it)->getNextHop())
             break;

         // Swap items
         tempSource = *it;
         *it = *backit;
         *backit = tempSource;

         --it;
         --backit;
    }
}*/

/*
void EigrpIpv4TopologyTable::moveRouteBack(EigrpRouteSource<IPv4Address> *source)
{
    RouteVector::iterator it, nextit;
    EigrpRouteSource<IPv4Address> *tempSource = nullptr;

    // Get iterator of source
    if ((it = std::find(routeVec.begin(), routeVec.end(), source)) == routeVec.end())
    {
        return;
    }

    // Move source behind all successors
    for (nextit = it + 1; nextit != routeVec.end(); it++, nextit ++)
    {
         if ((*it)->getRouteId() != (*nextit)->getRouteId())
             break;

         if (!(*nextit)->isSuccessor())
             break;

         // Swap items
         tempSource = *it;
         *it = *nextit;
         *nextit = tempSource;
    }
}*/

/**
 * Removes neighbor form the table, but the record still exists.
 */
template<typename IPAddress>
EigrpRouteSource<IPAddress> *EigrpTopologyTable<IPAddress>::removeRoute(EigrpRouteSource<IPAddress> *source)
{
    typename RouteVector::iterator it;

    if ((it = find(routeVec, source)) != routeVec.end()) {
        removeRoute(it);
        return source;
    }

    return nullptr;
}

template<typename IPAddress>
void EigrpTopologyTable<IPAddress>::delayedRemove(int neighId)
{
    typename RouteVector::iterator it;

    for (it = routeVec.begin(); it != routeVec.end();) {
        if ((*it)->getDelayedRemove() == neighId)
            it = removeRoute(it);
        else
            ++it;
    }
}

/**
 * Deletes invalid sources.
 */
template<typename IPAddress>
void EigrpTopologyTable<IPAddress>::purgeTable()
{
    typename RouteVector::iterator it;

    for (it = routeVec.begin(); it != routeVec.end();) {
        if (!(*it)->isValid()) { // Remove invalid source from TT
            it = removeRoute(it);
        }
        else
            ++it;
    }
}

template<typename IPAddress>
typename std::vector<EigrpRouteSource<IPAddress> *>::iterator EigrpTopologyTable<IPAddress>::removeRoute(typename RouteVector::iterator routeIt)
{
    EigrpRoute<IPAddress> *route = nullptr;
    EigrpRouteSource<IPAddress> *source = nullptr;

    source = *routeIt;
    route = source->getRouteInfo();

    EV_DEBUG << "EIGRP: remove route source " << route->getRouteAddress();
    EV_DEBUG << " via " << source->getNextHop() << " from TT" << endl;

    if (route->getRefCnt() == 1 && !route->isActive() && route->getNumSentMsgs() == 0) {
        EV_DEBUG << "EIGRP: remove route info " << route->getRouteAddress() << ", it does not have any source" << endl;
        removeRouteInfo(route);
        delete route;
    }
    else {
        route->decrementRefCnt();
    }

    delete source;
    return routeVec.erase(routeIt);
}

template<typename IPAddress>
EigrpRouteSource<IPAddress> *EigrpTopologyTable<IPAddress>::findRouteById(int sourceId)
{
    typename RouteVector::iterator it;
    for (it = routeVec.begin(); it != routeVec.end(); it++) {
        if ((*it)->getSourceId() == sourceId && (*it)->isValid())
            return *it;
    }
    return nullptr;
}

template<typename IPAddress>
EigrpRouteSource<IPAddress> *EigrpTopologyTable<IPAddress>::findRouteByNextHop(int routeId, int nextHopId)
{
    typename RouteVector::iterator it;
    for (it = routeVec.begin(); it != routeVec.end(); it++) {
        if ((*it)->getRouteId() == routeId && (*it)->getNexthopId() == nextHopId && (*it)->isValid())
            return *it;
    }
    return nullptr;
}

template<typename IPAddress>
EigrpRouteSource<IPAddress> *EigrpTopologyTable<IPAddress>::findOrCreateRoute(const IPAddress& routeAddr, const IPAddress& routeMask, const Ipv4Address& routerId,
                                                                              eigrp::EigrpInterface *eigrpIface, int nextHopId, bool *sourceNew)
{
    EigrpRoute<IPAddress> *route = nullptr;
    EigrpRouteSource<IPAddress> *source = nullptr;
    (*sourceNew) = false;

    // Find source of route with given next hop ID
    source = findRoute(routeAddr, routeMask, nextHopId);
    if (source == nullptr) {
        if ((route = findRouteInfo(routeAddr, routeMask)) == nullptr) { // Create route
            route = new EigrpRoute<IPAddress>(routeAddr, routeMask);
            addRouteInfo(route);
        }
        // Create source of route
        source = new EigrpRouteSource<IPAddress>(eigrpIface->getInterfaceId(), eigrpIface->getInterfaceName(), nextHopId, route->getRouteId(), route);
        source->setOriginator(routerId);
        (*sourceNew) = true;
        addRoute(source);
    }

    return source;
}

template<typename IPAddress>
EigrpRoute<IPAddress> *EigrpTopologyTable<IPAddress>::removeRouteInfo(EigrpRoute<IPAddress> *route)
{
    typename RouteInfoVector::iterator it;

    if ((it = find(routeInfoVec, route)) != routeInfoVec.end()) {
        routeInfoVec.erase(it);
        return route;
    }

    return nullptr;
}

template<typename IPAddress>
EigrpRoute<IPAddress> *EigrpTopologyTable<IPAddress>::findRouteInfo(const IPAddress& routeAddr, const IPAddress& routeMask)
{
    typename RouteInfoVector::iterator it;

    for (it = routeInfoVec.begin(); it != routeInfoVec.end(); it++) {
        if ((*it)->getRouteAddress() == routeAddr && (*it)->getRouteMask() == routeMask)
            return *it;
    }

    return nullptr;
}

template<typename IPAddress>
EigrpRoute<IPAddress> *EigrpTopologyTable<IPAddress>::findRouteInfoById(int routeId)
{
    typename RouteInfoVector::iterator it;

    for (it = routeInfoVec.begin(); it != routeInfoVec.end(); it++) {
        if ((*it)->getRouteId() == routeId)
            return *it;
    }

    return nullptr;
}

template class EigrpTopologyTable<Ipv4Address>;

#ifndef DISABLE_EIGRP_IPV6
template class EigrpTopologyTable<Ipv6Address>;
#endif /* DISABLE_EIGRP_IPV6 */

} // namespace eigrp
} // namespace inet

