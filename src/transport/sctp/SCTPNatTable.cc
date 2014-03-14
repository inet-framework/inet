//
// Copyright (C) 2008 Irene Ruengeler
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
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

#include "SCTPNatTable.h"
#include "NotifierConsts.h"
#include "SCTPAssociation.h"

uint32 SCTPNatTable::nextEntryNumber = 0;


Define_Module( SCTPNatTable );


SCTPNatTable::SCTPNatTable()
{

}

SCTPNatTable::~SCTPNatTable()
{
    for (uint32 i=0; i<natEntries.size(); i++)
        delete natEntries[i];
}

SCTPNatEntry* SCTPNatTable::findNatEntry(IPvXAddress srcAddr, uint16 srcPrt, IPvXAddress destAddr, uint16 destPrt, uint32 globalVtag)
{
    // linear search is OK because normally we don't have many interfaces and this func is rarely called
    Enter_Method_Silent();
    for (SCTPNatEntryTable::iterator i=natEntries.begin(); i!=natEntries.end(); ++i)
        if ((*i)->getLocalAddress()==srcAddr && (*i)->getLocalPort()==srcPrt &&
                (*i)->getGlobalAddress()==destAddr && (*i)->getGlobalPort()==destPrt && (*i)->getGlobalVTag()==globalVtag)
        {
            return *i;
        }
    return NULL;
}

SCTPNatEntry* SCTPNatTable::getEntry(IPvXAddress globalAddr, uint16 globalPrt, IPvXAddress nattedAddr, uint16 nattedPrt, uint32 localVtag)
{
    // linear search is OK because normally we don't have many interfaces and this func is rarely called
    Enter_Method_Silent();
    for (SCTPNatEntryTable::iterator i=natEntries.begin(); i!=natEntries.end(); ++i)
        if ((*i)->getGlobalAddress()==globalAddr && (*i)->getGlobalPort()==globalPrt &&
                (*i)->getNattedAddress()==nattedAddr && (*i)->getNattedPort()==nattedPrt &&
                (*i)->getLocalVTag()==localVtag)
            //  (*i)->getGlobalVTag()==localVtag)
        {
            return *i;
        }
    return NULL;
}

SCTPNatEntry* SCTPNatTable::getSpecialEntry(IPvXAddress globalAddr, uint16 globalPrt, IPvXAddress nattedAddr, uint16 nattedPrt)
{
    // linear search is OK because normally we don't have many interfaces and this func is rarely called
    Enter_Method_Silent();

    for (SCTPNatEntryTable::iterator i=natEntries.begin(); i!=natEntries.end(); ++i)
    {
        if ((*i)->getGlobalAddress()==globalAddr && (*i)->getGlobalPort()==globalPrt &&
                (*i)->getNattedAddress()==nattedAddr && (*i)->getNattedPort()==nattedPrt &&
                (*i)->getGlobalVTag()==0)
        {
            return *i;
        }
    }
    return NULL;
}

SCTPNatEntry* SCTPNatTable::getLocalInitEntry(IPvXAddress globalAddr, uint16 localPrt, uint16 globalPrt)
{
    // linear search is OK because normally we don't have many interfaces and this func is rarely called
    Enter_Method_Silent();
    for (SCTPNatEntryTable::iterator i=natEntries.begin(); i!=natEntries.end(); ++i)
        if ((*i)->getGlobalAddress()==globalAddr && (*i)->getGlobalPort()==localPrt &&
                (*i)->getLocalPort()==globalPrt)
        {
            return *i;
        }
    return NULL;
}

SCTPNatEntry* SCTPNatTable::getLocalEntry(IPvXAddress globalAddr, uint16 localPrt, uint16 globalPrt, uint32 localVtag)
{
    // linear search is OK because normally we don't have many interfaces and this func is rarely called
    Enter_Method_Silent();
    for (SCTPNatEntryTable::iterator i=natEntries.begin(); i!=natEntries.end(); ++i)
        if ((*i)->getGlobalAddress()==globalAddr && (*i)->getGlobalPort()==localPrt &&
                (*i)->getLocalPort()==globalPrt && (*i)->getLocalVTag()==localVtag)
        {
            return *i;
        }
    return NULL;
}

void SCTPNatTable::removeEntry(SCTPNatEntry* entry)
{
    Enter_Method_Silent();
    for (SCTPNatEntryTable::iterator i=natEntries.begin(); i!=natEntries.end(); ++i)
        if (((*i)->getGlobalAddress()==entry->getGlobalAddress() && (*i)->getGlobalPort()==entry->getGlobalPort() &&
                (*i)->getLocalPort()==entry->getLocalPort() && (*i)->getLocalVTag()==entry->getLocalVTag())
                || (((*i)->getLocalAddress()==entry->getGlobalAddress() && (*i)->getLocalPort()==entry->getGlobalPort() &&
                        (*i)->getGlobalPort()==entry->getLocalPort() && (*i)->getGlobalVTag()==entry->getLocalVTag())))
        {
            natEntries.erase(i);
            return;
        }
}

void SCTPNatTable::printNatTable()
{
    for (SCTPNatEntryTable::iterator i=natEntries.begin(); i!=natEntries.end(); ++i)
    {
        sctpEV3 << "localAddr:" << (*i)->getLocalAddress() << "  globalAddr:" << (*i)->getGlobalAddress() << "  localPort:" << (*i)->getLocalPort() << "  globalPort:" << (*i)->getGlobalPort() << "  nattedAddr:" << (*i)->getNattedAddress() << "  nattedPort:" << (*i)->getNattedPort() << "  localVtag:" << (*i)->getLocalVTag() << "  globalVtag:" << (*i)->getGlobalVTag() << "\n";
    }
}

SCTPNatEntry::SCTPNatEntry()
{
    localAddress = IPvXAddress("0.0.0.0");
    globalAddress = IPvXAddress("0.0.0.0");
    nattedAddress = IPvXAddress("0.0.0.0");
    localPort = 0;
    globalPort = 0;
    nattedPort = 0;
    globalVtag = 0;
    localVtag = 0;
}

SCTPNatEntry::~SCTPNatEntry()
{

}
