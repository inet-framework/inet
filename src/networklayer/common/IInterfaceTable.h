//
// Copyright (C) 2008 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_IINTERFACETABLE_H
#define __INET_IINTERFACETABLE_H

#include "inet/common/INETDefs.h"

#include "inet/networklayer/common/L3Address.h"
#include "inet/networklayer/common/InterfaceEntry.h"    // not strictly required, but clients will need it anyway

namespace inet {

struct MulticastGroup
{
    L3Address multicastAddr;
    int interfaceId;

    MulticastGroup(L3Address multicastAddr, int interfaceId) : multicastAddr(multicastAddr), interfaceId(interfaceId) {}
};

typedef std::vector<MulticastGroup> MulticastGroupList;

/**
 * A C++ interface to abstract the functionality of InterfaceTable.
 * Referring to InterfaceTable via this interface makes it possible to
 * transparently replace InterfaceTable with a different implementation,
 * without any change to the base INET.
 *
 * @see InterfaceTable, InterfaceEntry
 */
class INET_API IInterfaceTable
{
    friend class InterfaceEntry;    // so that it can call interfaceChanged()

  protected:
    // called from InterfaceEntry
    virtual void interfaceChanged(simsignal_t signalID, const InterfaceEntryChangeDetails *details) = 0;

  public:
    virtual ~IInterfaceTable() {}

    /**
     * Module path name
     */
    virtual std::string getFullPath() const = 0;

    /**
     * Returns the host or router this interface table lives in.
     */
    virtual cModule *getHostModule() = 0;

    /**
     * Checks if the address is a local one, i.e. one of the host's.
     */
    virtual bool isLocalAddress(const L3Address& address) const = 0;

    /**
     * Checks if the address is on the network of one of the interfaces,
     * but not local.
     */
    virtual bool isNeighborAddress(const L3Address& address) const = 0;

    /**
     * Returns an interface given by its address. Returns NULL if not found.
     */
    virtual InterfaceEntry *findInterfaceByAddress(const L3Address& address) const = 0;

    /**
     * Adds an interface. The entry->getInterfaceModule() will be used
     * to discover and fill in getNetworkLayerGateIndex(), getNodeOutputGateId(),
     * and getNodeInputGateId() in InterfaceEntry. It should be NULL if this is
     * a virtual interface (e.g. loopback).
     */
    virtual void addInterface(InterfaceEntry *entry) = 0;

    /**
     * Deletes the given interface from the table. Indices of existing
     * interfaces (see getInterface(int)) may change. It is an error if
     * the given interface is not in the table.
     */
    virtual void deleteInterface(InterfaceEntry *entry) = 0;

    /**
     * Returns the number of interfaces.
     */
    virtual int getNumInterfaces() = 0;

    /**
     * Returns the InterfaceEntry specified by an index 0..numInterfaces-1.
     * Throws an error if index is out of range.
     *
     * Note that this index is NOT the same as interfaceId! Indices are
     * not guaranteed to stay the same after interface addition/deletion,
     * so cannot be used to reliably identify the interface. Use interfaceId
     * to refer to interfaces from other modules or from messages/packets.
     */
    virtual InterfaceEntry *getInterface(int pos) = 0;

    /**
     * Returns an interface by its Id. Ids are guaranteed to be invariant
     * to interface deletions/additions. Returns NULL if there is no such
     * interface (This allows detecting stale IDs without raising an error.)
     */
    virtual InterfaceEntry *getInterfaceById(int id) = 0;

    /**
     * Returns the biggest interface Id.
     */
    virtual int getBiggestInterfaceId() = 0;

    /**
     * Returns an interface given by its getNodeOutputGateId().
     * Returns NULL if not found.
     */
    virtual InterfaceEntry *getInterfaceByNodeOutputGateId(int id) = 0;

    /**
     * Returns an interface given by its getNodeInputGateId().
     * Returns NULL if not found.
     */
    virtual InterfaceEntry *getInterfaceByNodeInputGateId(int id) = 0;

    /**
     * Returns an interface given by its getNetworkLayerGateIndex().
     * Returns NULL if not found.
     */
    virtual InterfaceEntry *getInterfaceByNetworkLayerGateIndex(int index) = 0;

    /**
     * Returns an interface by one of its component module (e.g. PPP).
     * Returns NULL if not found.
     */
    virtual InterfaceEntry *getInterfaceByInterfaceModule(cModule *ifmod) = 0;

    /**
     * Returns an interface given by its name. Returns NULL if not found.
     */
    virtual InterfaceEntry *getInterfaceByName(const char *name) = 0;

    /**
     * Returns the first interface with the isLoopback flag set.
     * (If there's no loopback, it returns NULL -- but this
     * should never happen because InterfaceTable itself registers a
     * loopback interface on startup.)
     */
    virtual InterfaceEntry *getFirstLoopbackInterface() = 0;

    /**
     * Returns the first multicast capable interface.
     * If there is no such interface, then returns NULL.
     */
    virtual InterfaceEntry *getFirstMulticastInterface() = 0;

    /**
     * Returns all multicast group address, with it's interfaceId
     */
    virtual MulticastGroupList collectMulticastGroups() = 0;
};

} // namespace inet

#endif // ifndef __INET_IINTERFACETABLE_H

