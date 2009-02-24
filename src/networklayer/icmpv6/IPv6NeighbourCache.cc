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


#include "IPv6NeighbourCache.h"


std::ostream& operator<<(std::ostream& os, const IPv6NeighbourCache::Key& e)
{
    return os << "if=" << e.interfaceID << " " << e.address; //FIXME try printing interface name
}

std::ostream& operator<<(std::ostream& os, const IPv6NeighbourCache::Neighbour& e)
{
    os << e.macAddress;
    if (e.isRouter) os << " ROUTER";
    if (e.isDefaultRouter) os << " defaultRtr";
    os << " " << IPv6NeighbourCache::stateName(e.reachabilityState);
    os << " reachabilityExp:"  << e.reachabilityExpires;
    if (e.numProbesSent) os << " probesSent:" << e.numProbesSent;
    if (e.isRouter) os << " rtrExp:" << e.routerExpiryTime;
    return os;
}

IPv6NeighbourCache::IPv6NeighbourCache()
{
    WATCH_MAP(neighbourMap);
}

IPv6NeighbourCache::Neighbour *IPv6NeighbourCache::lookup(const IPv6Address& addr, int interfaceID)
{
    Key key(addr, interfaceID);
    NeighbourMap::iterator i = neighbourMap.find(key);
    return i==neighbourMap.end() ? NULL : &(i->second);
}

const IPv6NeighbourCache::Key *IPv6NeighbourCache::lookupKeyAddr(Key& key)
{
    NeighbourMap::iterator i = neighbourMap.find(key);
    return &(i->first);
}

IPv6NeighbourCache::Neighbour *IPv6NeighbourCache::addNeighbour(const IPv6Address& addr, int interfaceID)
{
    Key key(addr, interfaceID);
    ASSERT(neighbourMap.find(key)==neighbourMap.end()); // entry must not exist yet
    Neighbour& nbor = neighbourMap[key];

    nbor.nceKey = lookupKeyAddr(key);//a ptr that links to the key.-WEI for convenience.
    nbor.isRouter = false;
    nbor.isDefaultRouter = false;
    nbor.reachabilityState = INCOMPLETE;
    nbor.reachabilityExpires = 0;
    nbor.numProbesSent = 0;
    nbor.nudTimeoutEvent = NULL;
    nbor.numOfARNSSent = 0;
    nbor.routerExpiryTime = 0;
    return &nbor;
}

IPv6NeighbourCache::Neighbour *IPv6NeighbourCache::addNeighbour(const IPv6Address& addr, int interfaceID, MACAddress macAddress)
{
    Key key(addr, interfaceID);
    ASSERT(neighbourMap.find(key)==neighbourMap.end()); // entry must not exist yet
    Neighbour& nbor = neighbourMap[key];

    nbor.nceKey = lookupKeyAddr(key);//a ptr that links to the key.-WEI for convenience.
    nbor.macAddress = macAddress;
    nbor.isRouter = false;
    nbor.isDefaultRouter = false;
    nbor.reachabilityState = STALE;
    nbor.reachabilityExpires = 0;
    nbor.numProbesSent = 0;
    nbor.nudTimeoutEvent = NULL;
    nbor.routerExpiryTime = 0;
    return &nbor;
}

/** Creates and initializes a router entry (isRouter=isDefaultRouter=true), state=INCOMPLETE. */
IPv6NeighbourCache::Neighbour *IPv6NeighbourCache::addRouter(const IPv6Address& addr, int interfaceID, simtime_t expiryTime)
{
    Key key(addr, interfaceID);
    ASSERT(neighbourMap.find(key)==neighbourMap.end()); // entry must not exist yet
    Neighbour& nbor = neighbourMap[key];

    nbor.nceKey = lookupKeyAddr(key);//a ptr that links to the key.-WEI for convenience.
    nbor.isRouter = true;
    nbor.isDefaultRouter = true;//FIXME: a router may advertise itself it self as a router but not as a default one.-WEI
    nbor.reachabilityState = INCOMPLETE;
    nbor.reachabilityExpires = 0;
    nbor.numProbesSent = 0;
    nbor.nudTimeoutEvent = NULL;
    nbor.routerExpiryTime = expiryTime;
    return &nbor;
}

/** Creates and initializes a router entry (isRouter=isDefaultRouter=true), MAC address and state=STALE. */
IPv6NeighbourCache::Neighbour *IPv6NeighbourCache::addRouter(const IPv6Address& addr, int interfaceID, MACAddress macAddress, simtime_t expiryTime)
{
    Key key(addr, interfaceID);
    ASSERT(neighbourMap.find(key)==neighbourMap.end()); // entry must not exist yet
    Neighbour& nbor = neighbourMap[key];

    nbor.nceKey = lookupKeyAddr(key);//a ptr that links to the key.-WEI for convenience.
    nbor.macAddress = macAddress;
    nbor.isRouter = true;
    nbor.isDefaultRouter = true;
    nbor.reachabilityState = STALE;
    nbor.reachabilityExpires = 0;
    nbor.numProbesSent = 0;
    nbor.nudTimeoutEvent = NULL;

    nbor.routerExpiryTime = expiryTime;
    return &nbor;
}

void IPv6NeighbourCache::remove(const IPv6Address& addr, int interfaceID)
{
    Key key(addr, interfaceID);
    NeighbourMap::iterator it = neighbourMap.find(key);
    ASSERT(it!=neighbourMap.end()); // entry must exist
    delete it->second.nudTimeoutEvent;
    neighbourMap.erase(it);
}

void IPv6NeighbourCache::remove(NeighbourMap::iterator it)
{
    delete it->second.nudTimeoutEvent;
    neighbourMap.erase(it);
}

const char *IPv6NeighbourCache::stateName(ReachabilityState state)
{
    switch (state)
    {
        case INCOMPLETE: return "INCOMPLETE";
        case REACHABLE:  return "REACHABLE";
        case STALE:      return "STALE";
        case DELAY:      return "DELAY";
        case PROBE:      return "PROBE";
        default:         return "???";
    }
}

