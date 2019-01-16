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

#ifndef __INET_IPV6NEIGHBOURCACHE_H
#define __INET_IPV6NEIGHBOURCACHE_H

#include <map>
#include <vector>

#include "inet/common/packet/Packet.h"
#include "inet/linklayer/common/MacAddress.h"
#include "inet/networklayer/contract/ipv6/Ipv6Address.h"

namespace inet {

/**
 * Ipv6 Neighbour Cache (RFC 2461 Neighbor Discovery for Ipv6).
 * Used internally by the Ipv6NeighbourDiscovery simple module.
 *
 * This is just a plain container class -- the Ipv6NeighbourDiscovery
 * module is free to manipulate the contents of the Neighbour entries
 * as it pleases.
 *
 * NOTE: Ipv6NeighbourCache also stores the Default Router List.
 * A router becomes a default router by calling
 * getDefaultRouterList().add(router) and stops to be a default router
 * after getDefaultRouterList().remove(router) has been called.
 * References to default routers are stored in a circular list to
 * ease round-robin selection.
 */
class INET_API Ipv6NeighbourCache
{
  public:
    typedef std::vector<Packet *> MsgPtrVector;    // TODO verify this is really needed --Andras

    /** Neighbour's reachability state */
    enum ReachabilityState { INCOMPLETE, REACHABLE, STALE, DELAY, PROBE };

    /**
     * Key into neighbour cache. Note that address does NOT identify the
     * neighbour itself, because link-local addresses are in common use
     * between routers.
     */
    struct Key
    {
        Ipv6Address address;
        int interfaceID;
        Key(Ipv6Address addr, int ifaceID) { address = addr; interfaceID = ifaceID; }
        bool operator<(const Key& b) const
        {
            return interfaceID == b.interfaceID ? address < b.address : interfaceID < b.interfaceID;
        }
    };

    /** Stores a neighbour (or router) entry */
    struct Neighbour
    {
        // Neighbour info
        const Key *nceKey = nullptr;    // points back to the key that links to this NCE
        MacAddress macAddress;
        bool isRouter = false;
        bool isHomeAgent = false;    //is the router also a Home Agent (RFC 3775-MIPv6)...Zarrar Yousaf 09.03.07

        // Neighbour Unreachability Detection variables
        ReachabilityState reachabilityState = static_cast<ReachabilityState>(-1);
        simtime_t reachabilityExpires;    // reachabilityLastConfirmed+reachableTime
        short numProbesSent = 0;
        cMessage *nudTimeoutEvent = nullptr;    // DELAY or PROBE timer

        //WEI-We could have a separate AREntry in the ND module.
        //But we should merge those information in the neighbour cache for a
        //cleaner solution. if reachability state is INCOMPLETE, it means that
        //addr resolution is being performed for this NCE.
        unsigned int numOfARNSSent = 0;
        cMessage *arTimer = nullptr;    //Address Resolution self-message timer
        MsgPtrVector pendingPackets;    //ptrs to queued packets associated with this NCE
        Ipv6Address nsSrcAddr;    //the src addr that was used to send the previous NS

        // Router variables.
        // NOTE: we only store lifetime expiry. Other Router Advertisement
        // fields (reachableTime, retransTimer, MTU) update the interface
        // configuration (Ipv6InterfaceData). We won't have a timer message
        // for router lifetime; instead, we'll check the expirytime every time
        // we bump into a router entry (as nexthop in dest cache, or during
        // default router selection
        simtime_t routerExpiryTime;    // time when router lifetime expires

        // for double-linked list of default routers, see DefaultRouterList
        Neighbour *prevDefaultRouter = nullptr;
        Neighbour *nextDefaultRouter = nullptr;

        // is it on the Default Router List?
        bool isDefaultRouter() const { return prevDefaultRouter && nextDefaultRouter; }

        Neighbour() {}
    };

    // Design note: we could have polymorphic entries in the neighbour cache
    // (i.e. a separate Router class subclassed from Neighbour), but then
    // we'd have to store in the map pointers to dynamically allocated
    // Neighbour structs, instead of the data directly. As long as expiryTime
    // is the only router-specific field, polymorphic entries don't pay off
    // because of the overhead caused by 'new'.

    /** The std::map underlying the Neighbour Cache data structure */
    typedef std::map<Key, Neighbour> NeighbourMap;

    // cyclic double-linked list of default routers
    class DefaultRouterList
    {
      public:
        class iterator
        {
            friend class DefaultRouterList;

          private:
            Neighbour *start;
            Neighbour *current;
            iterator(Neighbour *start) : start(start), current(start) {}

          public:
            iterator(const iterator& other) : start(other.start), current(other.current) {}
            Neighbour& operator*() { return *current; }
            iterator& operator++()    /*prefix*/ { current = current->nextDefaultRouter == start ? nullptr : current->nextDefaultRouter; return *this; }
            iterator operator++(int)    /*postfix*/ { iterator tmp(*this); operator++(); return tmp; }
            bool operator==(const iterator& rhs) const { return current == rhs.current; }
            bool operator!=(const iterator& rhs) const { return !(*this == rhs); }
        };

      private:
        Neighbour *head;

      public:
        DefaultRouterList() : head(nullptr) {}
        void clear() { head = nullptr; }
        Neighbour *getHead() const { return head; }
        void setHead(Neighbour& router) { ASSERT(router.isDefaultRouter()); head = &router; }
        void add(Neighbour& router);
        void remove(Neighbour& router);
        iterator begin() { return iterator(head); }
        iterator end() { return iterator(nullptr); }
    };

  protected:
    cSimpleModule& neighbourDiscovery;    // for cancelAndDelete() calls
    NeighbourMap neighbourMap;
    DefaultRouterList defaultRouterList;

  public:
    Ipv6NeighbourCache(cSimpleModule& neighbourDiscovery);
    virtual ~Ipv6NeighbourCache() {}

    /** Returns a neighbour entry, or nullptr. */
    virtual Neighbour *lookup(const Ipv6Address& addr, int interfaceID);

    /** Experimental code. */
    virtual const Key *lookupKeyAddr(Key& key);

    DefaultRouterList& getDefaultRouterList() { return defaultRouterList; }

    /** For iteration on the internal std::map */
    NeighbourMap::iterator begin() { return neighbourMap.begin(); }

    /** For iteration on the internal std::map */
    NeighbourMap::iterator end() { return neighbourMap.end(); }

    /** Creates and initializes a neighbour entry with isRouter=false, state=INCOMPLETE. */
    //TODO merge into next one (using default arg)
    virtual Neighbour *addNeighbour(const Ipv6Address& addr, int interfaceID);

    /** Creates and initializes a neighbour entry with isRouter=false, MAC address and state=STALE. */
    virtual Neighbour *addNeighbour(const Ipv6Address& addr, int interfaceID,
            MacAddress macAddress);

    /** Creates and initializes a router entry (isRouter=isDefaultRouter=true), MAC address and state=STALE. */
    virtual Neighbour *addRouter(const Ipv6Address& addr, int interfaceID,
            MacAddress macAddress, simtime_t expiryTime, bool isHomeAgent = false);    // added HA flag, 3.9.07 - CB

    /** Deletes the given neighbour from the cache. */
    virtual void remove(const Ipv6Address& addr, int interfaceID);

    /** Set status of all neighbours on given interface to state PROBE. */
    // Added by CB
    virtual void invalidateEntriesForInterfaceID(int interfaceID);

    /** Set status of all neighbours to state PROBE. */
    // Added by CB
    virtual void invalidateAllEntries();

    /** Deletes the given neighbour from the cache. */
    virtual void remove(NeighbourMap::iterator it);

    /** Returns the name of the given state as string */
    static const char *stateName(ReachabilityState state);
};

} // namespace inet

#endif // ifndef __INET_IPV6NEIGHBOURCACHE_H

