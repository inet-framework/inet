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
    displayString().setTagArg("t",0,buf);
}

void InterfaceTable::handleMessage(cMessage *msg)
{
    opp_error("This module doesn't process messages");
}

void InterfaceTable::receiveChangeNotification(int, cPolymorphic *)
{
    // nothing needed here at the moment
}

//---

InterfaceEntry *InterfaceTable::interfaceAt(int pos)
{
    if (pos<0 || pos>=(int)interfaces.size())
        opp_error("interfaceAt(): nonexistent interface %d", pos);
    return interfaces[pos];
}

void InterfaceTable::addInterface(InterfaceEntry *entry, cModule *ifmod)
{
    // check name is unique
    if (interfaceByName(entry->name())!=NULL)
        opp_error("addInterface(): interface '%s' already registered", entry->name());

    // insert
    entry->_interfaceId = interfaces.size();
    interfaces.push_back(entry);

    // fill in gates
    if (!ifmod)
        return; // logical interface

    // ifmod is something like "host.eth[1].mac"; climb up to find "host.eth[1]" from it
    cModule *host = parentModule();
    while (ifmod && ifmod->parentModule()!=host)
        ifmod = ifmod->parentModule();
    if (!ifmod)
        opp_error("addInterface(): specified module is not in this host/router");

    // find networkLayer as well
    cModule *nwlayer = host->submodule("networkLayer");
    if (!nwlayer) nwlayer = host->submodule("networkLayer6");
    if (!nwlayer)
        opp_error("addInterface(): cannot find network layer as 'networkLayer' or 'networkLayer6' submodule of %s", host->fullPath().c_str());

    // find gates connected to host / network layer
    cGate *nwlayerInGate, *nwlayerOutGate;
    for (int i=0; i<ifmod->gates(); i++)
    {
        cGate *g = ifmod->gate(i);
        if (!g) continue;
        if (g->type()=='O' && g->toGate() && g->toGate()->ownerModule()==host)
            entry->setNodeOutputGateId(g->toGate()->id());
        if (g->type()=='I' && g->fromGate() && g->fromGate()->ownerModule()==host)
            entry->setNodeInputGateId(g->fromGate()->id());
        if (g->type()=='O' && g->toGate() && g->toGate()->ownerModule()==nwlayer)
            nwlayerInGate = g->toGate();
        if (g->type()=='I' && g->fromGate() && g->fromGate()->ownerModule()==nwlayer)
            nwlayerOutGate = g->fromGate();
    }
    // consistency checks
    if (!nwlayerInGate || !nwlayerOutGate ||
        !nwlayerInGate->isName("ifIn") || !nwlayerOutGate->isName("ifOut") ||
        nwlayerInGate->index()!=nwlayerOutGate->index())
        opp_error("addInterface(): interface must be connected to network layer's ifIn[]/ifOut[] gates of the same index");
    entry->setNetworkLayerGateIndex(nwlayerInGate->index());
}

/*
void InterfaceTable::deleteInterface(InterfaceEntry *entry)
{
    // TBD ask routing tables if they refer to this interface

    InterfaceVector::iterator i = std::find(interfaces.begin(), interfaces.end(), entry);
    if (i==interfaces.end())
        opp_error("deleteInterface(): interface '%s' not found in interface table", entry->name.c_str());

    interfaces.erase(i);
    delete entry;
}
*/

InterfaceEntry *InterfaceTable::interfaceByNodeOutputGateId(int id)
{
    // linear search is OK because normally we have don't have many interfaces and this func is rarely called
    Enter_Method_Silent();
    for (InterfaceVector::iterator i=interfaces.begin(); i!=interfaces.end(); ++i)
        if ((*i)->nodeOutputGateId()==id)
            return *i;
    return NULL;
}

InterfaceEntry *InterfaceTable::interfaceByNodeInputGateId(int id)
{
    // linear search is OK because normally we have don't have many interfaces and this func is rarely called
    Enter_Method_Silent();
    for (InterfaceVector::iterator i=interfaces.begin(); i!=interfaces.end(); ++i)
        if ((*i)->nodeInputGateId()==id)
            return *i;
    return NULL;
}

InterfaceEntry *InterfaceTable::interfaceByNetworkLayerGateIndex(int index)
{
    // linear search is OK because normally we have don't have many interfaces and this func is rarely called
    Enter_Method_Silent();
    for (InterfaceVector::iterator i=interfaces.begin(); i!=interfaces.end(); ++i)
        if ((*i)->networkLayerGateIndex()==index)
            return *i;
    return NULL;
}

InterfaceEntry *InterfaceTable::interfaceByName(const char *name)
{
    Enter_Method_Silent();

    if (!name)
        return NULL;
    for (InterfaceVector::iterator i=interfaces.begin(); i!=interfaces.end(); ++i)
        if (!strcmp(name, (*i)->name()))
            return *i;
    return NULL;
}

InterfaceEntry *InterfaceTable::firstLoopbackInterface()
{
    Enter_Method_Silent();

    for (InterfaceVector::iterator i=interfaces.begin(); i!=interfaces.end(); ++i)
        if ((*i)->isLoopback())
            return *i;
    return NULL;
}

