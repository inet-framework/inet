//
// Copyright (C) 2005 Andras Varga
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

#ifndef __INET_INTERFACETABLE_H
#define __INET_INTERFACETABLE_H

#include <vector>

#include "inet/common/INETDefs.h"

#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/common/InterfaceEntry.h"
#include "inet/common/lifecycle/ILifecycle.h"

namespace inet {

/**
 * Represents the interface table. This object has one instance per host
 * or router. It has methods to manage the interface table,
 * so one can access functionality similar to the "ifconfig" command.
 *
 * See the NED documentation for general overview.
 *
 * This is a simple module without gates, it requires function calls to it
 * (message handling does nothing). Methods are provided for reading and
 * updating the interface table.
 *
 * Interfaces are dynamically registered: at the start of the simulation,
 * every L2 module adds its own InterfaceEntry to the table; after that,
 * IPv4's IIPv4RoutingTable and IPv6's IPv6RoutingTable (an possibly, further
 * L3 protocols) add protocol-specific data on each InterfaceEntry
 * (see IPv4InterfaceData, IPv6InterfaceData, and InterfaceEntry::setIPv4Data(),
 * InterfaceEntry::setIPv6Data())
 *
 * Interfaces are represented by InterfaceEntry objects.
 *
 * When interfaces need to be reliably and efficiently identified from other
 * modules, interfaceIds should be used. They are better suited than pointers
 * because when an interface gets removed (see deleteInterface()), it is
 * often impossible/impractical to invalidate all pointers to it, and also
 * because pointers are not necessarily unique (a new InterfaceEntry may get
 * allocated exactly at the address of a previously deleted one).
 * Interface Ids are unique (Ids of removed interfaces are not issued again),
 * stale Ids can be detected, and they are also invariant to insertion/deletion.
 *
 * Clients can get notified about interface changes by subscribing to
 * the following signals on host module: NF_INTERFACE_CREATED,
 * NF_INTERFACE_DELETED, NF_INTERFACE_STATE_CHANGED, NF_INTERFACE_CONFIG_CHANGED.
 * State change gets fired for up/down events; all other changes fire as
 * config change.
 *
 * @see InterfaceEntry
 */

class INET_API InterfaceTable : public cSimpleModule, public IInterfaceTable, protected cListener, public ILifecycle
{
  protected:
    cModule *host;    // cached pointer

    // primary storage for interfaces: vector indexed by id; may contain NULLs;
    // slots are never reused to ensure id uniqueness
    typedef std::vector<InterfaceEntry *> InterfaceVector;
    InterfaceVector idToInterface;

    // fields to support getNumInterfaces() and getInterface(pos)
    int tmpNumInterfaces;    // caches number of non-nullptr elements of idToInterface; -1 if invalid
    InterfaceEntry **tmpInterfaceList;    // caches non-nullptr elements of idToInterface; nullptr if invalid

  protected:
    // displays summary above the icon
    virtual void updateDisplayString();

    // displays the interface IPv4/IPv6 address on the outgoing link that corresponds to the interface
    virtual void updateLinkDisplayString(InterfaceEntry *entry);

    // discover and store which nwlayer/host gates connect to this interface
    virtual void discoverConnectingGates(InterfaceEntry *entry);

    // called from InterfaceEntry
    virtual void interfaceChanged(simsignal_t signalID, const InterfaceEntryChangeDetails *details) override;

    // internal
    virtual void invalidateTmpInterfaceList();

    virtual void resetInterfaces();

  public:
    InterfaceTable();
    virtual ~InterfaceTable();
    virtual std::string getFullPath() const override { return cSimpleModule::getFullPath(); }

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;

    /**
     * Raises an error.
     */
    virtual void handleMessage(cMessage *) override;

  public:
    /**
     * Called by the signal handler whenever a change of a category
     * occurs to which this client has subscribed.
     */
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj DETAILS_ARG) override;

    /**
     * Returns the host or router this interface table lives in.
     */
    virtual cModule *getHostModule() override;

    /**
     * Checks if the address is a local one, i.e. one of the host's.
     */
    virtual bool isLocalAddress(const L3Address& address) const override;

    /**
     * Checks if the address is on the network of one of the interfaces,
     * but not local.
     */
    virtual bool isNeighborAddress(const L3Address& address) const override;

    /**
     * Returns an interface given by its address. Returns nullptr if not found.
     */
    virtual InterfaceEntry *findInterfaceByAddress(const L3Address& address) const override;

    /**
     * Adds an interface. The entry->getInterfaceModule() will be used
     * to discover and fill in getNetworkLayerGateIndex(), getNodeOutputGateId(),
     * and getNodeInputGateId() in InterfaceEntry. It should be nullptr if this is
     * a virtual interface (e.g. loopback).
     */
    virtual void addInterface(InterfaceEntry *entry) override;

    /**
     * Deletes the given interface from the table. Indices of existing
     * interfaces (see getInterface(int)) may change. It is an error if
     * the given interface is not in the table.
     */
    virtual void deleteInterface(InterfaceEntry *entry) override;

    /**
     * Returns the number of interfaces.
     */
    virtual int getNumInterfaces() override;

    /**
     * Returns the InterfaceEntry specified by an index 0..numInterfaces-1.
     * Throws an error if index is out of range.
     *
     * Note that this index is NOT the same as interfaceId! Indices are
     * not guaranteed to stay the same after interface addition/deletion,
     * so cannot be used to reliably identify the interface. Use interfaceId
     * to refer to interfaces from other modules or from messages/packets.
     */
    virtual InterfaceEntry *getInterface(int pos) override;

    /**
     * Returns an interface by its Id. Ids are guaranteed to be invariant
     * to interface deletions/additions. Returns nullptr if there is no such
     * interface (This allows detecting stale IDs without raising an error.)
     */
    virtual InterfaceEntry *getInterfaceById(int id) override;

    /**
     * Returns the biggest interface Id.
     */
    virtual int getBiggestInterfaceId() override;

    /**
     * Returns an interface given by its getNodeOutputGateId().
     * Returns nullptr if not found.
     */
    virtual InterfaceEntry *getInterfaceByNodeOutputGateId(int id) override;

    /**
     * Returns an interface given by its getNodeInputGateId().
     * Returns nullptr if not found.
     */
    virtual InterfaceEntry *getInterfaceByNodeInputGateId(int id) override;

    /**
     * Returns an interface given by its getNetworkLayerGateIndex().
     * Returns nullptr if not found.
     */
    virtual InterfaceEntry *getInterfaceByNetworkLayerGateIndex(int index) override;

    /**
     * Returns an interface by one of its component module (e.g. PPP).
     * Returns nullptr if not found.
     */
    virtual InterfaceEntry *getInterfaceByInterfaceModule(cModule *ifmod) override;

    /**
     * Returns an interface given by its name. Returns nullptr if not found.
     */
    virtual InterfaceEntry *getInterfaceByName(const char *name) override;

    /**
     * Returns the first interface with the isLoopback flag set.
     * (If there's no loopback, it returns nullptr -- but this
     * should never happen because InterfaceTable itself registers a
     * loopback interface on startup.)
     */
    virtual InterfaceEntry *getFirstLoopbackInterface() override;

    /**
     * Returns the first multicast capable interface.
     * If there is no such interface, then returns nullptr.
     */
    virtual InterfaceEntry *getFirstMulticastInterface() override;

    /**
     * ILifecycle method
     */
    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback) override;

    /**
     * Returns all multicast group address, with it's interfaceId
     */
    virtual MulticastGroupList collectMulticastGroups() override;
};

} // namespace inet

#endif // ifndef __INET_INTERFACETABLE_H

