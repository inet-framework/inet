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

#include <omnetpp.h>
#include "INETDefs.h"
#include "InterfaceEntry.h"  // not strictly required, but clients will need it anyway


/**
 * A C++ interface to abstract the functionality of IInterfaceTable.
 * Referring to IInterfaceTable via this interface makes it possible to
 * transparently replace IInterfaceTable with a different implementation,
 * without any change to the base INET.
 *
 * @see IInterfaceTable, InterfaceEntry
 */
class INET_API IInterfaceTable
{
    friend class InterfaceEntry;  // so that it can call interfaceChanged()

  protected:
    // called from InterfaceEntry
    virtual void interfaceChanged(InterfaceEntry *entry, int category) = 0;

  public:
    virtual ~IInterfaceTable() {}

    /**
     * Module path name
     */
    virtual std::string getFullPath() const = 0;

    /**
     * Adds an interface. The second argument should be a module which belongs
     * to the physical interface (e.g. PPP or EtherMac) -- it will be used
     * to discover and fill in getNetworkLayerGateIndex(), getNodeOutputGateId(),
     * and getNodeInputGateId() in InterfaceEntry. It should be NULL if this is
     * a virtual interface (e.g. loopback).
     */
    virtual void addInterface(InterfaceEntry *entry, cModule *ifmod) = 0;

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
     * Returns an interface given by its name. Returns NULL if not found.
     */
    virtual InterfaceEntry *getInterfaceByName(const char *name) = 0;

    /**
     * Returns the first interface with the isLoopback flag set.
     * (If there's no loopback, it returns NULL -- but this
     * should never happen because IInterfaceTable itself registers a
     * loopback interface on startup.)
     */
    virtual InterfaceEntry *getFirstLoopbackInterface() = 0;
};

#endif

