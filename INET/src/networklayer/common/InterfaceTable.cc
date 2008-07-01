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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <algorithm>
#include <sstream>

#include "InterfaceTable.h"
#include "NotifierConsts.h"


Define_Module( InterfaceTable );

std::ostream& operator<<(std::ostream& os, const InterfaceEntry& e)
{
    os << e.info();
    return os;
};


InterfaceTable::InterfaceTable()
{
}

InterfaceTable::~InterfaceTable()
{
    for (unsigned int i=0; i<interfaces.size(); i++)
        delete interfaces[i];
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
        WATCH_PTRVECTOR(interfaces);
        updateDisplayString();
    }
}

void InterfaceTable::updateDisplayString()
{
    if (!ev.isGUI())
        return;

    char buf[80];
    sprintf(buf, "%d interfaces", interfaces.size());
    getDisplayString().setTagArg("t",0,buf);
}

void InterfaceTable::handleMessage(cMessage *msg)
{
    opp_error("This module doesn't process messages");
}

void InterfaceTable::receiveChangeNotification(int category, cPolymorphic *details)
{
    // nothing needed here at the moment
    Enter_Method_Silent();
    printNotificationBanner(category, details);
}

//---

InterfaceEntry *InterfaceTable::getInterface(int pos)
{
    if (pos==-1) // -1 is commonly used as "none"
        return NULL;
    if (pos<0 || pos>=(int)interfaces.size())
        opp_error("getInterface(): nonexistent interface %d", pos);
    return interfaces[pos];
}

void InterfaceTable::addInterface(InterfaceEntry *entry, cModule *ifmod)
{
    // check name is unique
    if (getInterfaceByName(entry->getName())!=NULL)
        opp_error("addInterface(): interface '%s' already registered", entry->getName());

    // insert
    entry->interfaceId = interfaces.size();
    entry->setInterfaceTable(this);
    interfaces.push_back(entry);

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
        if (g->getType()=='O' && g->getToGate() && g->getToGate()->getOwnerModule()==host)
            entry->setNodeOutputGateId(g->getToGate()->getId());
        if (g->getType()=='I' && g->getFromGate() && g->getFromGate()->getOwnerModule()==host)
            entry->setNodeInputGateId(g->getFromGate()->getId());

        // find the gate index of networkLayer/networkLayer6/mpls that connects to this interface
        if (g->getType()=='O' && g->getToGate() && g->getToGate()->isName("ifIn"))
            nwlayerInGate = g->getToGate();
        if (g->getType()=='I' && g->getFromGate() && g->getFromGate()->isName("ifOut"))
            nwlayerOutGate = g->getFromGate();
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
    InterfaceVector::iterator i = std::find(interfaces.begin(), interfaces.end(), entry);
    if (i==interfaces.end())
        opp_error("deleteInterface(): interface '%s' not found in interface table", entry->getName());

    nb->fireChangeNotification(NF_INTERFACE_DELETED, entry);  // actually, only going to be deleted

    interfaces.erase(i);
    delete entry;
}

void InterfaceTable::interfaceConfigChanged(InterfaceEntry *entry)
{
    nb->fireChangeNotification(NF_INTERFACE_CONFIG_CHANGED, entry);
}

void InterfaceTable::interfaceStateChanged(InterfaceEntry *entry)
{
    nb->fireChangeNotification(NF_INTERFACE_STATE_CHANGED, entry);
}

InterfaceEntry *InterfaceTable::getInterfaceByNodeOutputGateId(int id)
{
    // linear search is OK because normally we have don't have many interfaces and this func is rarely called
    Enter_Method_Silent();
    for (InterfaceVector::iterator i=interfaces.begin(); i!=interfaces.end(); ++i)
        if ((*i)->getNodeOutputGateId()==id)
            return *i;
    return NULL;
}

InterfaceEntry *InterfaceTable::getInterfaceByNodeInputGateId(int id)
{
    // linear search is OK because normally we have don't have many interfaces and this func is rarely called
    Enter_Method_Silent();
    for (InterfaceVector::iterator i=interfaces.begin(); i!=interfaces.end(); ++i)
        if ((*i)->getNodeInputGateId()==id)
            return *i;
    return NULL;
}

InterfaceEntry *InterfaceTable::getInterfaceByNetworkLayerGateIndex(int index)
{
    // linear search is OK because normally we have don't have many interfaces and this func is rarely called
    Enter_Method_Silent();
    for (InterfaceVector::iterator i=interfaces.begin(); i!=interfaces.end(); ++i)
        if ((*i)->getNetworkLayerGateIndex()==index)
            return *i;
    return NULL;
}

InterfaceEntry *InterfaceTable::getInterfaceByName(const char *name)
{
    Enter_Method_Silent();

    if (!name)
        return NULL;
    for (InterfaceVector::iterator i=interfaces.begin(); i!=interfaces.end(); ++i)
        if (!strcmp(name, (*i)->getName()))
            return *i;
    return NULL;
}

InterfaceEntry *InterfaceTable::getFirstLoopbackInterface()
{
    Enter_Method_Silent();

    for (InterfaceVector::iterator i=interfaces.begin(); i!=interfaces.end(); ++i)
        if ((*i)->isLoopback())
            return *i;
    return NULL;
}

