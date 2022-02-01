//
// Copyright (C) 2008 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IINTERFACETABLE_H
#define __INET_IINTERFACETABLE_H

#include "inet/networklayer/common/L3Address.h"

namespace inet {

class NetworkInterface;
class NetworkInterfaceChangeDetails;

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
 * @see InterfaceTable, NetworkInterface
 */
class INET_API IInterfaceTable
{
    friend class NetworkInterface; // so that it can call interfaceChanged()

  protected:
    // called from NetworkInterface
    virtual void interfaceChanged(simsignal_t signalID, const NetworkInterfaceChangeDetails *details) = 0;

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
    virtual NetworkInterface *findInterfaceByAddress(const L3Address& address) const = 0;

    /**
     * Adds an interface. The entry->getInterfaceModule() will be used
     * to discover and fill in getNetworkLayerGateIndex(), getNodeOutputGateId(),
     * and getNodeInputGateId() in NetworkInterface. It should be nullptr if this is
     * a virtual interface (e.g. loopback).
     */
    virtual void addInterface(NetworkInterface *entry) = 0;

    /**
     * Deletes the given interface from the table. Indices of existing
     * interfaces (see getInterface(int)) may change. It is an error if
     * the given interface is not in the table.
     */
    virtual void deleteInterface(NetworkInterface *entry) = 0;

    /**
     * Returns the number of interfaces.
     */
    virtual int getNumInterfaces() const = 0;

    /**
     * Returns the NetworkInterface specified by an index 0..numInterfaces-1.
     * Throws an error if index is out of range.
     *
     * Note that this index is NOT the same as interfaceId! Indices are
     * not guaranteed to stay the same after interface addition/deletion,
     * so cannot be used to reliably identify the interface. Use interfaceId
     * to refer to interfaces from other modules or from messages/packets.
     */
    virtual NetworkInterface *getInterface(int pos) const = 0;

    /**
     * Returns an interface by its Id. Ids are guaranteed to be invariant
     * to interface deletions/additions. Returns nullptr if there is no such
     * interface (This allows detecting stale IDs without raising an error.)
     */
    virtual NetworkInterface *findInterfaceById(int id) const = 0;

    /**
     * Returns an interface by its Id. Ids are guaranteed to be invariant
     * to interface deletions/additions. Throws an error if there is no such
     * interface.
     */
    virtual NetworkInterface *getInterfaceById(int id) const = 0;

    /**
     * Returns the biggest interface Id.
     */
    virtual int getBiggestInterfaceId() const = 0;

    /**
     * Returns an interface given by its getNodeOutputGateId().
     * Returns nullptr if not found.
     */
    virtual NetworkInterface *findInterfaceByNodeOutputGateId(int id) const = 0;

    /**
     * Returns an interface given by its getNodeInputGateId().
     * Returns nullptr if not found.
     */
    virtual NetworkInterface *findInterfaceByNodeInputGateId(int id) const = 0;

    /**
     * Returns an interface by one of its component module (e.g. PPP).
     * Returns nullptr if not found.
     */
    virtual NetworkInterface *findInterfaceByInterfaceModule(cModule *ifmod) const = 0;

    /**
     * Returns an interface given by its name. Returns nullptr if not found.
     */
    virtual NetworkInterface *findInterfaceByName(const char *name) const = 0;

    /**
     * Returns the first interface with the isLoopback flag set.
     * If there's no loopback, it returns nullptr.
     */
    virtual NetworkInterface *findFirstLoopbackInterface() const = 0;

    /**
     * Returns the first interface with the isLoopback flag unset.
     * If there's no non-loopback, it returns nullptr.
     */

    virtual NetworkInterface *findFirstNonLoopbackInterface() const = 0;
    /**
     * Returns the first multicast capable interface.
     * If there is no such interface, then returns nullptr.
     */
    virtual NetworkInterface *findFirstMulticastInterface() const = 0;

    /**
     * Returns all multicast group address, with it's interfaceId
     */
    virtual MulticastGroupList collectMulticastGroups() const = 0;
};

} // namespace inet

#endif

