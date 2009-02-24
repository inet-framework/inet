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

#ifndef NEIGHBORCACHE_H
#define NEIGHBORCACHE_H

#include <map>
#include <vector>
#include <omnetpp.h>
#include "IPv6Address.h"
#include "MACAddress.h"


/**
 * IPv6 Neighbour Cache (RFC 2461 Neighbor Discovery for IPv6).
 * Used internally by the IPv6NeighbourDiscovery simple module.
 *
 * This is just a plain container class -- the IPv6NeighbourDiscovery
 * module is free to manipulate the contents of the Neighbour entries
 * as it pleases.
 *
 * NOTE: we don't keep a separate Default Router List, the Neighbour
 * Cache serves that purpose too. Removing an entry from the
 * Default Router List in our case is done by setting the isDefaultRouter
 * flag of the entry to false.
 */
class INET_API IPv6NeighbourCache
{
  public:
    typedef std::vector<cMessage*> MsgPtrVector;  // TODO verify this is really needed --Andras

    /** Neighbour's reachability state */
    enum ReachabilityState {INCOMPLETE, REACHABLE, STALE, DELAY, PROBE};

    /**
     * Key into neighbour cache. Note that address does NOT identify the
     * neighbour itself, because link-local addresses are in common use
     * between routers.
     */
    struct Key
    {
        IPv6Address address;
        int interfaceID;
        Key(IPv6Address addr, int ifaceID) {address=addr; interfaceID=ifaceID;}
        bool operator<(const Key& b) const {
            return interfaceID==b.interfaceID ? address<b.address : interfaceID<b.interfaceID;
        }
    };

    /** Stores a neighbour (or router) entry */
    struct Neighbour
    {
        // Neighbour info
        const Key *nceKey;//store a pointer back to the key that links to this NCE.-WEI
        MACAddress macAddress;
        bool isRouter;
        bool isDefaultRouter; // is it on the Default Router List?

        // Neighbour Unreachability Detection variables
        ReachabilityState reachabilityState;
        simtime_t reachabilityExpires; // reachabilityLastConfirmed+reachableTime
        short numProbesSent;
        cMessage *nudTimeoutEvent; // DELAY or PROBE timer

        //WEI-We could have a seperate AREntry in the ND module.
        //But we should merge those information in the neighbour cache for a
        //cleaner solution. if reachability state is INCOMPLETE, it means that
        //addr resolution is being performed for this NCE.
        int numOfARNSSent;
        cMessage *arTimer;//Address Resolution self-message timer
        MsgPtrVector pendingPackets; //ptrs to queued packets associated with this NCE
        IPv6Address nsSrcAddr;//the src addr that was used to send the previous NS

        // Router variables.
        // NOTE: we only store lifetime expiry. Other Router Advertisement
        // fields (reachableTime, retransTimer, MTU) update the interface
        // configuration (IPv6InterfaceData). We won't have a timer message
        // for router lifetime; instead, we'll check the expirytime every time
        // we bump into a router entry (as nexthop in dest cache, or during
        // default router selection
        simtime_t routerExpiryTime;   // time when router lifetime expires
    };

    // Design note: we could have polymorphic entries in the neighbour cache
    // (i.e. a separate Router class subclassed from Neighbour), but then
    // we'd have to store in the map pointers to dynamically allocated
    // Neighbour structs, instead of the data directly. As long as expiryTime
    // is the only router-specific field, polymorphic entries don't pay off
    // because of the overhead caused by 'new'.

    /** The std::map underlying the Neighbour Cache data structure */
    typedef std::map<Key,Neighbour> NeighbourMap;
    typedef NeighbourMap::iterator iterator;

  protected:
    NeighbourMap neighbourMap;

  public:
    IPv6NeighbourCache();
    virtual ~IPv6NeighbourCache() {}

    /** Returns a neighbour entry, or NULL. */
    virtual Neighbour *lookup(const IPv6Address& addr, int interfaceID);

    /** Experimental code. */
    virtual const Key *lookupKeyAddr(Key& key);

    /** For iteration on the internal std::map */
    iterator begin()  {return neighbourMap.begin();}

    /** For iteration on the internal std::map */
    iterator end()  {return neighbourMap.end();}

    /** Creates and initializes a neighbour entry with isRouter=false, state=INCOMPLETE. */
    //TODO merge into next one (using default arg)
    virtual Neighbour *addNeighbour(const IPv6Address& addr, int interfaceID);

    /** Creates and initializes a neighbour entry with isRouter=false, MAC address and state=STALE. */
    virtual Neighbour *addNeighbour(const IPv6Address& addr, int interfaceID,
                            MACAddress macAddress);

    /** Creates and initializes a router entry (isRouter=isDefaultRouter=true), state=INCOMPLETE. */
    //TODO merge into next one (using default arg)
    virtual Neighbour *addRouter(const IPv6Address& addr, int interfaceID,
                        simtime_t expiryTime);

    /** Creates and initializes a router entry (isRouter=isDefaultRouter=true), MAC address and state=STALE. */
    virtual Neighbour *addRouter(const IPv6Address& addr, int interfaceID,
                         MACAddress macAddress, simtime_t expiryTime);

    /** Deletes the given neighbour from the cache. */
    virtual void remove(const IPv6Address& addr, int interfaceID);

    /** Deletes the given neighbour from the cache. */
    virtual void remove(NeighbourMap::iterator it);

    /** Returns the name of the given state as string */
    static const char *stateName(ReachabilityState state);
};

#endif
