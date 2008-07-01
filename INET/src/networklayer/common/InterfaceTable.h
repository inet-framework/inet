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
// License along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#ifndef __INTERFACETABLE_H
#define __INTERFACETABLE_H

#include <vector>
#include <omnetpp.h>
#include "INETDefs.h"
#include "InterfaceEntry.h"
#include "NotificationBoard.h"


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
 * IPv4's RoutingTable and IPv6's RoutingTable6 (an possibly, further
 * L3 protocols) add protocol-specific data on each InterfaceEntry
 * (see IPv4InterfaceData, IPv6InterfaceData, and InterfaceEntry::setIPv4Data(),
 * InterfaceEntry::setIPv6Data())
 *
 * Interfaces are represented by InterfaceEntry objects.
 *
 * @see InterfaceEntry
 */
class INET_API InterfaceTable : public cSimpleModule, public INotifiable
{
    friend class InterfaceEntry;  // so that it can call interfaceConfigChanged()

  protected:
    NotificationBoard *nb; // cached pointer

    typedef std::vector<InterfaceEntry *> InterfaceVector;
    InterfaceVector interfaces;

  protected:
    // displays summary above the icon
    virtual void updateDisplayString();

    // discover and store which nwlayer/host gates connect to this interface
    virtual void discoverConnectingGates(InterfaceEntry *entry, cModule *ifmod);

    // called from InterfaceEntry
    virtual void interfaceConfigChanged(InterfaceEntry *entry);
    virtual void interfaceStateChanged(InterfaceEntry *entry);

  public:
    InterfaceTable();
    virtual ~InterfaceTable();

  protected:
    virtual int numInitStages() const {return 2;}
    virtual void initialize(int stage);

    /**
     * Raises an error.
     */
    virtual void handleMessage(cMessage *);

  public:
    /**
     * Called by the NotificationBoard whenever a change of a category
     * occurs to which this client has subscribed.
     */
    virtual void receiveChangeNotification(int category, cPolymorphic *details);

    /**
     * Adds an interface. The second argument should be a module which belongs
     * to the physical interface (e.g. PPP or EtherMac) -- it will be used
     * to discover and fill in getNetworkLayerGateIndex(), getNodeOutputGateId(),
     * and getNodeInputGateId() in InterfaceEntry. It should be NULL if this is
     * a virtual interface (e.g. loopback).
     */
    virtual void addInterface(InterfaceEntry *entry, cModule *ifmod);

    /**
     * Deletes the given interface from the table. Indices of existing
     * interfaces (see getInterface(int)) may change. It is an error if
     * the given interface is not in the table.
     */
    virtual void deleteInterface(InterfaceEntry *entry);

    /**
     * Returns the number of interfaces.
     */
    virtual int getNumInterfaces()  {return interfaces.size();}

    /**
     * Returns the InterfaceEntry specified by an index 0..numInterfaces-1.
     */
    virtual InterfaceEntry *getInterface(int pos);

    /**
     * Returns an interface given by its getNodeOutputGateId().
     * Returns NULL if not found.
     */
    virtual InterfaceEntry *getInterfaceByNodeOutputGateId(int id);

    /**
     * Returns an interface given by its getNodeInputGateId().
     * Returns NULL if not found.
     */
    virtual InterfaceEntry *getInterfaceByNodeInputGateId(int id);

    /**
     * Returns an interface given by its getNetworkLayerGateIndex().
     * Returns NULL if not found.
     */
    virtual InterfaceEntry *getInterfaceByNetworkLayerGateIndex(int index);

    /**
     * Returns an interface given by its name. Returns NULL if not found.
     */
    virtual InterfaceEntry *getInterfaceByName(const char *name);

    /**
     * Returns the first interface with the isLoopback flag set.
     * (If there's no loopback, it returns NULL -- but this
     * should never happen because InterfaceTable itself registers a
     * loopback interface on startup.)
     */
    virtual InterfaceEntry *getFirstLoopbackInterface();
};

#endif

