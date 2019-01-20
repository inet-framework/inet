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

#include <algorithm>
#include <ctype.h>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "inet/common/Simsignals.h"
#include "inet/transportlayer/sctp/SctpAssociation.h"
#include "inet/transportlayer/sctp/SctpNatTable.h"

namespace inet {
namespace sctp {

uint32 SctpNatTable::nextEntryNumber = 0;

Define_Module(SctpNatTable);

SctpNatTable::SctpNatTable()
{
}

SctpNatTable::~SctpNatTable()
{
    for (auto & elem : natEntries)
        delete(elem);
    natEntries.clear();
}

SctpNatEntry *SctpNatTable::findNatEntry(L3Address srcAddr, uint16 srcPrt, L3Address destAddr, uint16 destPrt, uint32 globalVtag)
{
    // linear search is OK because normally we don't have many interfaces and this func is rarely called
    Enter_Method_Silent();
    for (auto & elem : natEntries)
        if ((elem)->getLocalAddress() == srcAddr && (elem)->getLocalPort() == srcPrt &&
            (elem)->getGlobalAddress() == destAddr && (elem)->getGlobalPort() == destPrt && (elem)->getGlobalVTag() == globalVtag)
        {
            return elem;
        }
    return nullptr;
}

SctpNatEntry *SctpNatTable::getEntry(L3Address globalAddr, uint16 globalPrt, L3Address nattedAddr, uint16 nattedPrt, uint32 localVtag)
{
    // linear search is OK because normally we don't have many interfaces and this func is rarely called
    Enter_Method_Silent();
    for (auto & elem : natEntries)
        if ((elem)->getGlobalAddress() == globalAddr && (elem)->getGlobalPort() == globalPrt &&
            (elem)->getNattedAddress() == nattedAddr && (elem)->getNattedPort() == nattedPrt &&
            (elem)->getLocalVTag() == localVtag)
        //  (*i)->getGlobalVTag()==localVtag)
        {
            return elem;
        }
    return nullptr;
}

SctpNatEntry *SctpNatTable::getSpecialEntry(L3Address globalAddr, uint16 globalPrt, L3Address nattedAddr, uint16 nattedPrt)
{
    // linear search is OK because normally we don't have many interfaces and this func is rarely called
    Enter_Method_Silent();

    for (auto & elem : natEntries) {
        if ((elem)->getGlobalAddress() == globalAddr && (elem)->getGlobalPort() == globalPrt &&
            (elem)->getNattedAddress() == nattedAddr && (elem)->getNattedPort() == nattedPrt &&
            (elem)->getGlobalVTag() == 0)
        {
            return elem;
        }
    }
    return nullptr;
}

SctpNatEntry *SctpNatTable::getLocalInitEntry(L3Address globalAddr, uint16 localPrt, uint16 globalPrt)
{
    // linear search is OK because normally we don't have many interfaces and this func is rarely called
    Enter_Method_Silent();
    for (auto & elem : natEntries)
        if ((elem)->getGlobalAddress() == globalAddr && (elem)->getGlobalPort() == localPrt &&
            (elem)->getLocalPort() == globalPrt)
        {
            return elem;
        }
    return nullptr;
}

SctpNatEntry *SctpNatTable::getLocalEntry(L3Address globalAddr, uint16 localPrt, uint16 globalPrt, uint32 localVtag)
{
    // linear search is OK because normally we don't have many interfaces and this func is rarely called
    Enter_Method_Silent();
    for (auto & elem : natEntries)
        if ((elem)->getGlobalAddress() == globalAddr && (elem)->getGlobalPort() == localPrt &&
            (elem)->getLocalPort() == globalPrt && (elem)->getLocalVTag() == localVtag)
        {
            return elem;
        }
    return nullptr;
}

void SctpNatTable::removeEntry(SctpNatEntry *entry)
{
    Enter_Method_Silent();
    for (auto i = natEntries.begin(); i != natEntries.end(); ++i)
        if (((*i)->getGlobalAddress() == entry->getGlobalAddress() && (*i)->getGlobalPort() == entry->getGlobalPort() &&
             (*i)->getLocalPort() == entry->getLocalPort() && (*i)->getLocalVTag() == entry->getLocalVTag())
            || (((*i)->getLocalAddress() == entry->getGlobalAddress() && (*i)->getLocalPort() == entry->getGlobalPort() &&
                 (*i)->getGlobalPort() == entry->getLocalPort() && (*i)->getGlobalVTag() == entry->getLocalVTag())))
        {
            natEntries.erase(i);
            return;
        }
}

void SctpNatTable::printNatTable()
{
    for (auto & elem : natEntries) {
        EV_INFO << "localAddr:" << (elem)->getLocalAddress() << "  globalAddr:" << (elem)->getGlobalAddress() << "  localPort:" << (elem)->getLocalPort() << "  globalPort:" << (elem)->getGlobalPort() << "  nattedAddr:" << (elem)->getNattedAddress() << "  nattedPort:" << (elem)->getNattedPort() << "  localVtag:" << (elem)->getLocalVTag() << "  globalVtag:" << (elem)->getGlobalVTag() << "\n";
    }
}

SctpNatEntry::SctpNatEntry()
{
    localAddress = L3Address();
    globalAddress = L3Address();
    nattedAddress = L3Address();
    localPort = 0;
    globalPort = 0;
    nattedPort = 0;
    globalVtag = 0;
    localVtag = 0;
    entryNumber = 0;
}

SctpNatEntry::~SctpNatEntry()
{
}

} // namespace sctp
} // namespace inet

