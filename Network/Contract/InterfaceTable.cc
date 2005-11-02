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

void InterfaceTable::initialize(int stage)
{
    if (stage==0)
    {
        // get a pointer to the NotificationBoard module
        nb = NotificationBoardAccess().get();

        // register a loopback interface
        InterfaceEntry *ie = new InterfaceEntry();
        ie->setName("lo0");
        ie->setOutputPort(-1);
        ie->setMtu(3924);
        ie->setLoopback(true);
        addInterface(ie);
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

void InterfaceTable::addInterface(InterfaceEntry *entry)
{
    // check name and outputPort are unique
    if (interfaceByName(entry->name())!=NULL)
        opp_error("addInterface(): interface '%s' already registered", entry->name());
    if (entry->outputPort()!=-1 && interfaceByPortNo(entry->outputPort())!=NULL)
        opp_error("addInterface(): interface with outputPort=%d already registered", entry->outputPort());

    // insert
    entry->_interfaceId = interfaces.size();
    interfaces.push_back(entry);
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

int InterfaceTable::numInterfaceGates()
{
    // linear search is OK because normally we have don't have many interfaces (1..4, rarely more)
    int max = -1;
    for (InterfaceVector::iterator i=interfaces.begin(); i!=interfaces.end(); ++i)
        if ((*i)->outputPort()>max)
            max = (*i)->outputPort();
    return max+1;
}

InterfaceEntry *InterfaceTable::interfaceByPortNo(int portNo)
{
    Enter_Method_Silent();

    // linear search is OK because normally we have don't have many interfaces (1..4, rarely more)
    for (InterfaceVector::iterator i=interfaces.begin(); i!=interfaces.end(); ++i)
        if ((*i)->outputPort()==portNo)
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

