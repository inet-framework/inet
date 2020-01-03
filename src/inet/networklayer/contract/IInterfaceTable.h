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
#include "inet/networklayer/common/InterfaceEntry.h"    // not strictly required, but clients will need it anyway
#include "inet/networklayer/common/L3Address.h"

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
    virtual cModule *getHostModule() const = 0;

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
     * Returns an interface given by its address. Returns nullptr if not found.
     */
    virtual InterfaceEntry *findInterfaceByAddress(const L3Address& address) const = 0;

    /**
     * Adds an interface. The entry->getInterfaceModule() will be used
     * to discover and fill in getNetworkLayerGateIndex(), getNodeOutputGateId(),
     * and getNodeInputGateId() in InterfaceEntry. It should be nullptr if this is
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
    virtual int getNumInterfaces() const = 0;

    /**
     * Returns the InterfaceEntry specified by an index 0..numInterfaces-1.
     * Throws an error if index is out of range.
     *
     * Note that this index is NOT the same as interfaceId! Indices are
     * not guaranteed to stay the same after interface addition/deletion,
     * so cannot be used to reliably identify the interface. Use interfaceId
     * to refer to interfaces from other modules or from messages/packets.
     */
    virtual InterfaceEntry *getInterface(int pos) const = 0;

    /**
     * Returns an interface by its Id. Ids are guaranteed to be invariant
     * to interface deletions/additions. Returns nullptr if there is no such
     * interface (This allows detecting stale IDs without raising an error.)
     */
    virtual InterfaceEntry *findInterfaceById(int id) const = 0;

    /**
     * Returns an interface by its Id. Ids are guaranteed to be invariant
     * to interface deletions/additions. Throws an error if there is no such
     * interface.
     */
    virtual InterfaceEntry *getInterfaceById(int id) const = 0;

    /**
     * Returns the biggest interface Id.
     */
    virtual int getBiggestInterfaceId() const = 0;

    /**
     * Returns an interface given by its getNodeOutputGateId().
     * Returns nullptr if not found.
     */
    virtual InterfaceEntry *findInterfaceByNodeOutputGateId(int id) const = 0;

    /**
     * Returns an interface given by its getNodeInputGateId().
     * Returns nullptr if not found.
     */
    virtual InterfaceEntry *findInterfaceByNodeInputGateId(int id) const = 0;

    /**
     * Returns an interface by one of its component module (e.g. PPP).
     * Returns nullptr if not found.
     */
    virtual InterfaceEntry *findInterfaceByInterfaceModule(cModule *ifmod) const = 0;

    /**
     * Returns an interface given by its name. Returns nullptr if not found.
     */
    virtual InterfaceEntry *findInterfaceByName(const char *name) const = 0;

    /**
     * Returns the first interface with the isLoopback flag set.
     * If there's no loopback, it returns nullptr.
     */
    virtual InterfaceEntry *findFirstLoopbackInterface() const = 0;

    /**
     * Returns the first interface with the isLoopback flag unset.
     * If there's no non-loopback, it returns nullptr.
     */

    virtual InterfaceEntry *findFirstNonLoopbackInterface() const = 0;
    /**
     * Returns the first multicast capable interface.
     * If there is no such interface, then returns nullptr.
     */
    virtual InterfaceEntry *findFirstMulticastInterface() const = 0;

    /**
     * Returns all multicast group address, with it's interfaceId
     */
    virtual MulticastGroupList collectMulticastGroups() const = 0;
};

} // namespace inet

#endif // ifndef __INET_IINTERFACETABLE_H

