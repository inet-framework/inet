//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_INTERFACETABLE_H
#define __INET_INTERFACETABLE_H

#include <vector>

#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/lifecycle/OperationalBase.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/networklayer/contract/IInterfaceTable.h"

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
 * every L2 module adds its own NetworkInterface to the table; after that,
 * Ipv4's IIpv4RoutingTable and Ipv6's Ipv6RoutingTable (an possibly, further
 * L3 protocols) add protocol-specific data on each NetworkInterface
 * (see Ipv4InterfaceData, Ipv6InterfaceData, and NetworkInterface::setIPv4Data(),
 * NetworkInterface::setIPv6Data())
 *
 * Interfaces are represented by NetworkInterface objects.
 *
 * When interfaces need to be reliably and efficiently identified from other
 * modules, interfaceIds should be used. They are better suited than pointers
 * because when an interface gets removed (see deleteInterface()), it is
 * often impossible/impractical to invalidate all pointers to it, and also
 * because pointers are not necessarily unique (a new NetworkInterface may get
 * allocated exactly at the address of a previously deleted one).
 * Interface Ids are unique (Ids of removed interfaces are not issued again),
 * stale Ids can be detected, and they are also invariant to insertion/deletion.
 *
 * Clients can get notified about interface changes by subscribing to
 * the following signals on host module: interfaceCreatedSignal,
 * interfaceDeletedSignal, interfaceStateChangedSignal, interfaceConfigChangedSignal.
 * State change gets fired for up/down events; all other changes fire as
 * config change.
 *
 * @see NetworkInterface
 */

class INET_API InterfaceTable : public OperationalBase, public IInterfaceTable, protected cListener
{
  protected:
    cModule *host; // cached pointer

    // primary storage for interfaces: vector indexed by id; may contain NULLs;
    // slots are never reused to ensure id uniqueness
    typedef std::vector<NetworkInterface *> InterfaceVector;
    InterfaceVector idToInterface;

    // fields to support getNumInterfaces() and getInterface(pos)
    mutable int tmpNumInterfaces; // caches number of non-nullptr elements of idToInterface; -1 if invalid
    mutable NetworkInterface **tmpInterfaceList; // caches non-nullptr elements of idToInterface; nullptr if invalid

  protected:
    // displays summary above the icon
    virtual void refreshDisplay() const override;

    // displays the interface Ipv4/Ipv6 address on the outgoing link that corresponds to the interface
    virtual void updateLinkDisplayString(NetworkInterface *entry) const;

    // discover and store which nwlayer/host gates connect to this interface
    virtual void discoverConnectingGates(NetworkInterface *entry);

    // called from NetworkInterface
    virtual void interfaceChanged(simsignal_t signalID, const NetworkInterfaceChangeDetails *details) override;

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
    virtual void handleMessageWhenUp(cMessage *) override;

  public:
    /**
     * Called by the signal handler whenever a change of a category
     * occurs to which this client has subscribed.
     */
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;

    /**
     * Returns the host or router this interface table lives in.
     */
    virtual cModule *getHostModule() const override;

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
    virtual NetworkInterface *findInterfaceByAddress(const L3Address& address) const override;

    /**
     * Adds an interface. The entry->getInterfaceModule() will be used
     * to discover and fill in getNetworkLayerGateIndex(), getNodeOutputGateId(),
     * and getNodeInputGateId() in NetworkInterface. It should be nullptr if this is
     * a virtual interface (e.g. loopback).
     */
    virtual void addInterface(NetworkInterface *entry) override;

    /**
     * Deletes the given interface from the table. Indices of existing
     * interfaces (see getInterface(int)) may change. It is an error if
     * the given interface is not in the table.
     */
    virtual void deleteInterface(NetworkInterface *entry) override;

    /**
     * Returns the number of interfaces.
     */
    virtual int getNumInterfaces() const override;

    /**
     * Returns the NetworkInterface specified by an index 0..numInterfaces-1.
     * Throws an error if index is out of range.
     *
     * Note that this index is NOT the same as interfaceId! Indices are
     * not guaranteed to stay the same after interface addition/deletion,
     * so cannot be used to reliably identify the interface. Use interfaceId
     * to refer to interfaces from other modules or from messages/packets.
     */
    virtual NetworkInterface *getInterface(int pos) const override;

    /**
     * Returns an interface by its Id. Ids are guaranteed to be invariant
     * to interface deletions/additions. Returns nullptr if there is no such
     * interface (This allows detecting stale IDs without raising an error.)
     */
    virtual NetworkInterface *findInterfaceById(int id) const override;

    /**
     * Returns an interface by its Id. Ids are guaranteed to be invariant
     * to interface deletions/additions. Throws an error if there is no such
     * interface.
     */
    virtual NetworkInterface *getInterfaceById(int id) const override;

    /**
     * Returns the biggest interface Id.
     */
    virtual int getBiggestInterfaceId() const override;

    /**
     * Returns an interface given by its getNodeOutputGateId().
     * Returns nullptr if not found.
     */
    virtual NetworkInterface *findInterfaceByNodeOutputGateId(int id) const override;

    /**
     * Returns an interface given by its getNodeInputGateId().
     * Returns nullptr if not found.
     */
    virtual NetworkInterface *findInterfaceByNodeInputGateId(int id) const override;

    /**
     * Returns an interface by one of its component module (e.g. PPP).
     * Returns nullptr if not found.
     */
    virtual NetworkInterface *findInterfaceByInterfaceModule(cModule *ifmod) const override;

    /**
     * Returns an interface given by its name. Returns nullptr if not found.
     */
    virtual NetworkInterface *findInterfaceByName(const char *name) const override;

    /**
     * Returns the first interface with the isLoopback flag set.
     * If there's no loopback, it returns nullptr.
     */
    virtual NetworkInterface *findFirstLoopbackInterface() const override;

    /**
     * Returns the first interface with the isLoopback flag unset.
     * If there's no non-loopback, it returns nullptr.
     */
    virtual NetworkInterface *findFirstNonLoopbackInterface() const override;

    /**
     * Returns the first multicast capable interface.
     * If there is no such interface, then returns nullptr.
     */
    virtual NetworkInterface *findFirstMulticastInterface() const override;

    /**
     * Lifecycle method
     */
    virtual bool isInitializeStage(int stage) const override { return stage == INITSTAGE_LINK_LAYER; }
    virtual bool isModuleStartStage(int stage) const override { return stage == ModuleStartOperation::STAGE_LINK_LAYER; }
    virtual bool isModuleStopStage(int stage) const override { return stage == ModuleStopOperation::STAGE_LINK_LAYER; }
    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

    /**
     * Returns all multicast group address, with it's interfaceId
     */
    virtual MulticastGroupList collectMulticastGroups() const override;
};

} // namespace inet

#endif

