//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
// Copyright (C) OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// Babel routing protocol (RFC 6126) ported from the ANSAINET project.
// Original authors: Vit Rek, Vladimir Vesely (Brno University of Technology).
//

#include "inet/routing/babel/BabelTopologyTable.h"

#include <sstream>

namespace inet {
namespace babel {

BabelRoute::~BabelRoute()
{
    deleteETimer();
    deleteBETimer();
}

std::string BabelRoute::str() const
{
    std::stringstream out;
    out << (selected ? "> " : "  ");
    out << prefix;
    if (neighbour != nullptr)
        out << " NH:" << nexthop;
    else
        out << " local";
    out << " metric:" << metric();
    out << " orig:" << originator;
    if (neighbour != nullptr) {
        out << " from:" << neighbour->getAddress();
        out << " RD:" << rdistance;
    }
    if (rtentry)
        out << " (in RT)";
    return out.str();
}

uint16_t BabelRoute::metric() const
{
    if (neighbour != nullptr) {
        if (neighbour->getCost() == COST_INF)
            return COST_INF;
        unsigned int m = rdistance.getMetric() + neighbour->getCost();
        return m >= COST_INF ? COST_INF : m;
    }
    return rdistance.getMetric();
}

void BabelRoute::resetETimer() { resetTimer(expiryTimer, defval::ROUTE_EXPIRY_INTERVAL_MULT * CStoS(updateInterval)); }
void BabelRoute::resetETimer(double delay) { resetTimer(expiryTimer, delay); }
void BabelRoute::resetBETimer() { resetTimer(befExpiryTimer, (defval::ROUTE_EXPIRY_INTERVAL_MULT * CStoS(updateInterval) * 7.0) / 8.0); }
void BabelRoute::resetBETimer(double delay) { resetTimer(befExpiryTimer, delay); }
void BabelRoute::deleteETimer() { deleteTimer(&expiryTimer); }
void BabelRoute::deleteBETimer() { deleteTimer(&befExpiryTimer); }

BabelTopologyTable::~BabelTopologyTable()
{
    removeRoutes();
}

void BabelTopologyTable::removeRoutes()
{
    for (auto route : routes)
        delete route;
    routes.clear();
}

BabelRoute *BabelTopologyTable::findRoute(const netPrefix<L3Address>& p, BabelNeighbour *n, const rid& orig)
{
    for (auto route : routes)
        if (route->getPrefix() == p && route->getNeighbour() == n && route->getOriginator() == orig)
            return route;
    return nullptr;
}

BabelRoute *BabelTopologyTable::findRoute(const netPrefix<L3Address>& p, BabelNeighbour *n)
{
    for (auto route : routes)
        if (route->getPrefix() == p && route->getNeighbour() == n)
            return route;
    return nullptr;
}

BabelRoute *BabelTopologyTable::findSelectedRoute(const netPrefix<L3Address>& p)
{
    for (auto route : routes)
        if (route->getPrefix() == p && route->getSelected())
            return route;
    return nullptr;
}

bool BabelTopologyTable::containShorterCovRoute(const netPrefix<L3Address>& p)
{
    for (auto route : routes) {
        const netPrefix<L3Address>& rp = route->getPrefix();
        if (rp.getAddr().getType() == p.getAddr().getType() && rp.getLen() < p.getLen()) {
            if (rp.getAddr().getType() == L3Address::IPv6) {
                if (rp.getAddr().toIpv6() == p.getAddr().toIpv6().getPrefix(rp.getLen()))
                    return true;
            }
            else {
                if (rp.getAddr().toIpv4() == p.getAddr().toIpv4().doAnd(Ipv4Address::makeNetmask(rp.getLen())))
                    return true;
            }
        }
    }
    return false;
}

BabelRoute *BabelTopologyTable::findRouteNotNH(const netPrefix<L3Address>& p, const L3Address& nh)
{
    for (auto route : routes)
        if (route->getPrefix() == p && route->getNextHop() != nh)
            return route;
    return nullptr;
}

BabelRoute *BabelTopologyTable::addRoute(BabelRoute *route)
{
    ASSERT(route != nullptr);
    BabelRoute *intable = findRoute(route->getPrefix(), route->getNeighbour());
    if (intable != nullptr)
        return intable;
    routes.push_back(route);
    return route;
}

bool BabelTopologyTable::retractRoutesOnIface(BabelInterface *iface)
{
    bool changed = false;
    for (auto route : routes) {
        if (route->getNeighbour() != nullptr && route->getNeighbour()->getInterface() == iface) {
            route->getRDistance().setMetric(COST_INF);
            changed = true;
        }
    }
    return changed;
}

bool BabelTopologyTable::removeRoute(BabelRoute *route)
{
    for (auto it = routes.begin(); it != routes.end(); ++it) {
        if (*it == route) {
            delete *it;
            routes.erase(it);
            return true;
        }
    }
    return false;
}

} // namespace babel
} // namespace inet
