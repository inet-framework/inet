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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <algorithm>
#include <sstream>

#include "InterfaceTable.h"
#include "NotifierConsts.h"


Define_Module( InterfaceTable );

#define INTERFACEIDS_START  100

std::ostream& operator<<(std::ostream& os, const InterfaceEntry& e)
{
    os << e.info();
    return os;
};


InterfaceTable::InterfaceTable()
{
    tmpNumInterfaces = -1;
    tmpInterfaceList = NULL;
}

InterfaceTable::~InterfaceTable()
{
    for (int i=0; i < (int)idToInterface.size(); i++)
        delete idToInterface[i];
    delete [] tmpInterfaceList;
}

void InterfaceTable::initialize(int stage)
{
    if (stage==0)
    {
        // get a pointer to the NotificationBoard module
        nb = NotificationBoardAccess().get();

        // register a loopback interface
        InterfaceEntry *ie = new InterfaceEntry();
        ie->setName("lo0");
        ie->setMtu(3924);
        ie->setLoopback(true);
        addInterface(ie, NULL);
    }
    else if (stage==1)
    {
        WATCH_PTRVECTOR(idToInterface);
        updateDisplayString();
    }
}

void InterfaceTable::updateDisplayString()
{
    if (!ev.isGUI())
        return;

    char buf[80];
    sprintf(buf, "%d interfaces", getNumInterfaces());
    getDisplayString().setTagArg("t",0,buf);
}

void InterfaceTable::handleMessage(cMessage *msg)
{
    opp_error("This module doesn't process messages");
}

void InterfaceTable::receiveChangeNotification(int category, const cPolymorphic *details)
{
    // nothing needed here at the moment
    Enter_Method_Silent();
    printNotificationBanner(category, details);
}

//---

int InterfaceTable::getNumInterfaces()
{
    if (tmpNumInterfaces == -1)
    {
        // count non-NULL elements
        int n = 0;
        int maxId = idToInterface.size();
        for (int i=0; i<maxId; i++)
            if (idToInterface[i])
                n++;
        tmpNumInterfaces = n;
    }

    return tmpNumInterfaces;
}

InterfaceEntry *InterfaceTable::getInterface(int pos)
{
    int n = getNumInterfaces(); // also fills tmpInterfaceList
    if (pos<0 || pos>=n)
        opp_error("getInterface(): interface index %d out of range 0..%d", pos, n-1);

    if (!tmpInterfaceList)
    {
        // collect non-NULL elements into tmpInterfaceList[]
        tmpInterfaceList = new InterfaceEntry *[n];
        int k = 0;
        int maxId = idToInterface.size();
        for (int i=0; i<maxId; i++)
            if (idToInterface[i])
                tmpInterfaceList[k++] = idToInterface[i];
    }

    return tmpInterfaceList[pos];
}

InterfaceEntry *InterfaceTable::getInterfaceById(int id)
{
    id -= INTERFACEIDS_START;
    return (id<0 || id>=(int)idToInterface.size()) ? NULL : idToInterface[id];
}

void InterfaceTable::addInterface(InterfaceEntry *entry, cModule *ifmod)
{
    // check name is unique
    if (getInterfaceByName(entry->getName())!=NULL)
        opp_error("addInterface(): interface '%s' already registered", entry->getName());

    // insert
    entry->setInterfaceId(INTERFACEIDS_START + idToInterface.size());
    entry->setInterfaceTable(this);
    idToInterface.push_back(entry);
    invalidateTmpInterfaceList();

    // fill in networkLayerGateIndex, nodeOutputGateId, nodeInputGateId
    if (ifmod)
        discoverConnectingGates(entry, ifmod);

    nb->fireChangeNotification(NF_INTERFACE_CREATED, entry);
}

void InterfaceTable::discoverConnectingGates(InterfaceEntry *entry, cModule *ifmod)
{
    // ifmod is something like "host.eth[1].mac"; climb up to find "host.eth[1]" from it
    cModule *host = getParentModule();
    while (ifmod && ifmod->getParentModule()!=host)
        ifmod = ifmod->getParentModule();
    if (!ifmod)
        opp_error("addInterface(): specified module is not in this host/router");

    // find gates connected to host / network layer
    cGate *nwlayerInGate=NULL, *nwlayerOutGate=NULL;
    for (GateIterator i(ifmod); !i.end(); i++)
    {
        cGate *g = i();
        if (!g) continue;

        // find the host/router's gates that internally connect to this interface
        if (g->getType()==cGate::OUTPUT && g->getNextGate() && g->getNextGate()->getOwnerModule()==host)
            entry->setNodeOutputGateId(g->getNextGate()->getId());
        if (g->getType()==cGate::INPUT && g->getPreviousGate() && g->getPreviousGate()->getOwnerModule()==host)
            entry->setNodeInputGateId(g->getPreviousGate()->getId());

        // find the gate index of networkLayer/networkLayer6/mpls that connects to this interface
        if (g->getType()==cGate::OUTPUT && g->getNextGate() && g->getNextGate()->isName("ifIn"))
            nwlayerInGate = g->getNextGate();
        if (g->getType()==cGate::INPUT && g->getPreviousGate() && g->getPreviousGate()->isName("ifOut"))
            nwlayerOutGate = g->getPreviousGate();
    }

    // consistency checks
    // note: we don't check nodeOutputGateId/nodeInputGateId, because wireless interfaces
    // are not connected to the host
    if (!nwlayerInGate || !nwlayerOutGate || nwlayerInGate->getIndex()!=nwlayerOutGate->getIndex())
        opp_error("addInterface(): interface must be connected to network layer's ifIn[]/ifOut[] gates of the same index");
    entry->setNetworkLayerGateIndex(nwlayerInGate->getIndex());
}

void InterfaceTable::deleteInterface(InterfaceEntry *entry)
{
    int id = entry->getInterfaceId();
    if (entry != getInterfaceById(id))
        opp_error("deleteInterface(): interface '%s' not found in interface table", entry->getName());

    nb->fireChangeNotification(NF_INTERFACE_DELETED, entry);  // actually, only going to be deleted

    idToInterface[id - INTERFACEIDS_START] = NULL;
    delete entry;
    invalidateTmpInterfaceList();
}

void InterfaceTable::invalidateTmpInterfaceList()
{
    tmpNumInterfaces = -1;
    delete[] tmpInterfaceList;
    tmpInterfaceList = NULL;
}

void InterfaceTable::interfaceChanged(InterfaceEntry *entry, int category)
{
    nb->fireChangeNotification(category, entry);
}

InterfaceEntry *InterfaceTable::getInterfaceByNodeOutputGateId(int id)
{
    // linear search is OK because normally we have don't have many interfaces and this func is rarely called
    Enter_Method_Silent();
    int n = idToInterface.size();
    for (int i=0; i<n; i++)
        if (idToInterface[i] && idToInterface[i]->getNodeOutputGateId()==id)
            return idToInterface[i];
    return NULL;
}

InterfaceEntry *InterfaceTable::getInterfaceByNodeInputGateId(int id)
{
    // linear search is OK because normally we have don't have many interfaces and this func is rarely called
    Enter_Method_Silent();
    int n = idToInterface.size();
    for (int i=0; i<n; i++)
        if (idToInterface[i] && idToInterface[i]->getNodeInputGateId()==id)
            return idToInterface[i];
    return NULL;
}

InterfaceEntry *InterfaceTable::getInterfaceByNetworkLayerGateIndex(int index)
{
    // linear search is OK because normally we have don't have many interfaces and this func is rarely called
    Enter_Method_Silent();
    int n = idToInterface.size();
    for (int i=0; i<n; i++)
        if (idToInterface[i] && idToInterface[i]->getNetworkLayerGateIndex()==index)
            return idToInterface[i];
    return NULL;
}

InterfaceEntry *InterfaceTable::getInterfaceByName(const char *name)
{
    Enter_Method_Silent();
    if (!name)
        return NULL;
    int n = idToInterface.size();
    for (int i=0; i<n; i++)
        if (idToInterface[i] && !strcmp(name, idToInterface[i]->getName()))
            return idToInterface[i];
    return NULL;
}

InterfaceEntry *InterfaceTable::getFirstLoopbackInterface()
{
    Enter_Method_Silent();
    int n = idToInterface.size();
    for (int i=0; i<n; i++)
        if (idToInterface[i] && idToInterface[i]->isLoopback())
            return idToInterface[i];
    return NULL;
}

