/**
 * Copyright (C) 2005 Andras Varga
 * Copyright (C) 2005 Wei Yang, Ng
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include "inet/networklayer/icmpv6/IPv6NeighbourCache.h"

namespace inet {

void IPv6NeighbourCache::DefaultRouterList::add(Neighbour& router)
{
    ASSERT(router.isRouter);
    ASSERT(!router.nextDefaultRouter);
    ASSERT(!router.prevDefaultRouter);

    if (head) {
        // insert before head
        head->prevDefaultRouter->nextDefaultRouter = &router;
        router.prevDefaultRouter = head;
        head->prevDefaultRouter = &router;
        router.nextDefaultRouter = head;
    }
    else {
        head = router.nextDefaultRouter = router.prevDefaultRouter = &router;
    }
}

void IPv6NeighbourCache::DefaultRouterList::remove(Neighbour& router)
{
    ASSERT(router.isDefaultRouter());
    if (router.nextDefaultRouter == &router) {
        head = NULL;
    }
    else {
        if (head == &router)
            head = router.nextDefaultRouter;
        router.nextDefaultRouter->prevDefaultRouter = router.prevDefaultRouter;
        router.prevDefaultRouter->nextDefaultRouter = router.nextDefaultRouter;
    }
    router.nextDefaultRouter = router.prevDefaultRouter = NULL;
}

std::ostream& operator<<(std::ostream& os, const IPv6NeighbourCache::Key& e)
{
    return os << "if=" << e.interfaceID << " " << e.address;    //FIXME try printing interface name
}

std::ostream& operator<<(std::ostream& os, const IPv6NeighbourCache::Neighbour& e)
{
    os << e.macAddress;
    if (e.isRouter)
        os << " ROUTER";
    if (e.isDefaultRouter())
        os << "DefaultRtr";
    if (e.isHomeAgent)
        os << " Home Agent";
    os << " " << IPv6NeighbourCache::stateName(e.reachabilityState);
    os << " reachabilityExp:" << e.reachabilityExpires;
    if (e.numProbesSent)
        os << " probesSent:" << e.numProbesSent;
    if (e.isRouter)
        os << " rtrExp:" << e.routerExpiryTime;
    return os;
}

IPv6NeighbourCache::IPv6NeighbourCache(cSimpleModule& neighbourDiscovery)
    : neighbourDiscovery(neighbourDiscovery)
{
    WATCH_MAP(neighbourMap);
}

IPv6NeighbourCache::Neighbour *IPv6NeighbourCache::lookup(const IPv6Address& addr, int interfaceID)
{
    Key key(addr, interfaceID);
    NeighbourMap::iterator i = neighbourMap.find(key);
    return i == neighbourMap.end() ? NULL : &(i->second);
}

const IPv6NeighbourCache::Key *IPv6NeighbourCache::lookupKeyAddr(Key& key)
{
    NeighbourMap::iterator i = neighbourMap.find(key);
    return &(i->first);
}

IPv6NeighbourCache::Neighbour *IPv6NeighbourCache::addNeighbour(const IPv6Address& addr, int interfaceID)
{
    Key key(addr, interfaceID);
    ASSERT(neighbourMap.find(key) == neighbourMap.end());    // entry must not exist yet
    Neighbour& nbor = neighbourMap[key];

    nbor.nceKey = lookupKeyAddr(key);
    nbor.isRouter = false;
    nbor.isHomeAgent = false;
    nbor.reachabilityState = INCOMPLETE;
    return &nbor;
}

IPv6NeighbourCache::Neighbour *IPv6NeighbourCache::addNeighbour(const IPv6Address& addr, int interfaceID, MACAddress macAddress)
{
    Key key(addr, interfaceID);
    ASSERT(neighbourMap.find(key) == neighbourMap.end());    // entry must not exist yet
    Neighbour& nbor = neighbourMap[key];

    nbor.nceKey = lookupKeyAddr(key);
    nbor.macAddress = macAddress;
    nbor.isRouter = false;
    nbor.isHomeAgent = false;
    nbor.reachabilityState = STALE;
    return &nbor;
}

/**
 * Creates and initializes a router entry (isRouter=isDefaultRouter=true), MAC address and state=STALE.
 *
 * Update by CB: Added an optional parameter which is false by default. Specifies whether a router is also a home agent.
 */
IPv6NeighbourCache::Neighbour *IPv6NeighbourCache::addRouter(const IPv6Address& addr,
        int interfaceID, MACAddress macAddress, simtime_t expiryTime, bool isHomeAgent)
{
    Key key(addr, interfaceID);
    ASSERT(neighbourMap.find(key) == neighbourMap.end());    // entry must not exist yet
    Neighbour& nbor = neighbourMap[key];

    nbor.nceKey = lookupKeyAddr(key);
    nbor.macAddress = macAddress;
    nbor.isRouter = true;
    nbor.isHomeAgent = isHomeAgent;
    nbor.reachabilityState = STALE;
    nbor.routerExpiryTime = expiryTime;

    defaultRouterList.add(nbor);

    return &nbor;
}

void IPv6NeighbourCache::remove(const IPv6Address& addr, int interfaceID)
{
    Key key(addr, interfaceID);
    NeighbourMap::iterator it = neighbourMap.find(key);
    ASSERT(it != neighbourMap.end());    // entry must exist
    neighbourDiscovery.cancelAndDelete(it->second.nudTimeoutEvent);
    it->second.nudTimeoutEvent = NULL;
    if (it->second.isDefaultRouter())
        defaultRouterList.remove(it->second);
    neighbourMap.erase(it);
}

void IPv6NeighbourCache::remove(NeighbourMap::iterator it)
{
    //delete it->second.nudTimeoutEvent;
    neighbourDiscovery.cancelAndDelete(it->second.nudTimeoutEvent);    // 20.9.07 - CB
    it->second.nudTimeoutEvent = NULL;
    if (it->second.isDefaultRouter())
        defaultRouterList.remove(it->second);
    neighbourMap.erase(it);
}

// Added by CB
void IPv6NeighbourCache::invalidateEntriesForInterfaceID(int interfaceID)
{
    for (NeighbourMap::iterator it = neighbourMap.begin(); it != neighbourMap.end(); it++) {
        if (it->first.interfaceID == interfaceID) {
            it->second.reachabilityState = PROBE;    // we make sure this neighbour is not used anymore in the future, unless reachability can be confirmed
            neighbourDiscovery.cancelAndDelete(it->second.nudTimeoutEvent);    // 20.9.07 - CB
            it->second.nudTimeoutEvent = NULL;
        }
    }
}

// Added by CB
void IPv6NeighbourCache::invalidateAllEntries()
{
    while (!neighbourMap.empty()) {
        NeighbourMap::iterator it = neighbourMap.begin();
        remove(it);
    }
    defaultRouterList.clear();

    /*
       int size = neighbourMap.size();
       EV << "size: " << size << endl;
       for (NeighbourMap::iterator it = neighbourMap.begin(); it != neighbourMap.end(); it++)
       {
        it->second.reachabilityState = PROBE; // we make sure this neighbour is not used anymore in the future, unless reachability can be confirmed
       }
     */
}

const char *IPv6NeighbourCache::stateName(ReachabilityState state)
{
    switch (state) {
        case INCOMPLETE:
            return "INCOMPLETE";

        case REACHABLE:
            return "REACHABLE";

        case STALE:
            return "STALE";

        case DELAY:
            return "DELAY";

        case PROBE:
            return "PROBE";

        default:
            return "???";
    }
}

} // namespace inet

