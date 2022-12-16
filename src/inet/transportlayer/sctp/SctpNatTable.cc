//
// Copyright (C) 2008 Irene Ruengeler
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/transportlayer/sctp/SctpNatTable.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <algorithm>
#include <sstream>

#include "inet/common/Simsignals.h"
#include "inet/transportlayer/sctp/SctpAssociation.h"

namespace inet {
namespace sctp {

Define_Module(SctpNatTable);

SctpNatTable::SctpNatTable()
{
}

SctpNatTable::~SctpNatTable()
{
    for (auto& elem : natEntries)
        delete elem;
    natEntries.clear();
}

SctpNatEntry *SctpNatTable::findNatEntry(L3Address srcAddr, uint16_t srcPrt, L3Address destAddr, uint16_t destPrt, uint32_t globalVtag)
{
    // linear search is OK because normally we don't have many interfaces and this func is rarely called
    Enter_Method("findNatEntry");
    for (auto& elem : natEntries)
        if ((elem)->getLocalAddress() == srcAddr && (elem)->getLocalPort() == srcPrt &&
            (elem)->getGlobalAddress() == destAddr && (elem)->getGlobalPort() == destPrt && (elem)->getGlobalVTag() == globalVtag)
        {
            return elem;
        }
    return nullptr;
}

SctpNatEntry *SctpNatTable::getEntry(L3Address globalAddr, uint16_t globalPrt, L3Address nattedAddr, uint16_t nattedPrt, uint32_t localVtag)
{
    // linear search is OK because normally we don't have many interfaces and this func is rarely called
    Enter_Method("getEntry");
    for (auto& elem : natEntries)
        if ((elem)->getGlobalAddress() == globalAddr && (elem)->getGlobalPort() == globalPrt &&
            (elem)->getNattedAddress() == nattedAddr && (elem)->getNattedPort() == nattedPrt &&
            (elem)->getLocalVTag() == localVtag)
//        (*i)->getGlobalVTag()==localVtag)
        {
            return elem;
        }
    return nullptr;
}

SctpNatEntry *SctpNatTable::getSpecialEntry(L3Address globalAddr, uint16_t globalPrt, L3Address nattedAddr, uint16_t nattedPrt)
{
    // linear search is OK because normally we don't have many interfaces and this func is rarely called
    Enter_Method("getSpecialEntry");

    for (auto& elem : natEntries) {
        if ((elem)->getGlobalAddress() == globalAddr && (elem)->getGlobalPort() == globalPrt &&
            (elem)->getNattedAddress() == nattedAddr && (elem)->getNattedPort() == nattedPrt &&
            (elem)->getGlobalVTag() == 0)
        {
            return elem;
        }
    }
    return nullptr;
}

SctpNatEntry *SctpNatTable::getLocalInitEntry(L3Address globalAddr, uint16_t localPrt, uint16_t globalPrt)
{
    // linear search is OK because normally we don't have many interfaces and this func is rarely called
    Enter_Method("getLocalInitEntry");
    for (auto& elem : natEntries)
        if ((elem)->getGlobalAddress() == globalAddr && (elem)->getGlobalPort() == localPrt &&
            (elem)->getLocalPort() == globalPrt)
        {
            return elem;
        }
    return nullptr;
}

SctpNatEntry *SctpNatTable::getLocalEntry(L3Address globalAddr, uint16_t localPrt, uint16_t globalPrt, uint32_t localVtag)
{
    // linear search is OK because normally we don't have many interfaces and this func is rarely called
    Enter_Method("getLocalEntry");
    for (auto& elem : natEntries)
        if ((elem)->getGlobalAddress() == globalAddr && (elem)->getGlobalPort() == localPrt &&
            (elem)->getLocalPort() == globalPrt && (elem)->getLocalVTag() == localVtag)
        {
            return elem;
        }
    return nullptr;
}

void SctpNatTable::removeEntry(SctpNatEntry *entry)
{
    Enter_Method("removeEntry");
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
    for (auto& elem : natEntries) {
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

