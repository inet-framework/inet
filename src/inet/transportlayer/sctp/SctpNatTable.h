//
// Copyright (C) 2008 Irene Ruengeler
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_SCTPNATTABLE_H
#define __INET_SCTPNATTABLE_H

#include <vector>

#include "inet/networklayer/common/L3Address.h"
#include "inet/transportlayer/sctp/SctpAssociation.h"

namespace inet {
namespace sctp {

class INET_API SctpNatEntry : public cObject
{
  protected:
    uint32_t entryNumber;
    L3Address localAddress;
    L3Address globalAddress;
    L3Address nattedAddress;
    uint16_t localPort;
    uint16_t globalPort;
    uint16_t nattedPort;
    uint32_t globalVtag;
    uint32_t localVtag;

  public:
    SctpNatEntry();
    ~SctpNatEntry();

    void setLocalAddress(L3Address addr) { localAddress = addr; }
    void setGlobalAddress(L3Address addr) { globalAddress = addr; }
    void setNattedAddress(L3Address addr) { nattedAddress = addr; }
    void setLocalPort(uint16_t port) { localPort = port; }
    void setGlobalPort(uint16_t port) { globalPort = port; }
    void setNattedPort(uint16_t port) { nattedPort = port; }
    void setGlobalVTag(uint32_t tag) { globalVtag = tag; }
    void setLocalVTag(uint32_t tag) { localVtag = tag; }
    void setEntryNumber(uint32_t number) { entryNumber = number; }

    L3Address getLocalAddress() { return localAddress; };
    L3Address getGlobalAddress() { return globalAddress; };
    L3Address getNattedAddress() { return nattedAddress; };
    uint16_t getLocalPort() { return localPort; };
    uint16_t getGlobalPort() { return globalPort; };
    uint16_t getNattedPort() { return nattedPort; };
    uint32_t getGlobalVTag() { return globalVtag; };
    uint32_t getLocalVTag() { return localVtag; };
};

class INET_API SctpNatTable : public cSimpleModule
{
  public:

    typedef std::vector<SctpNatEntry *> SctpNatEntryTable;

    SctpNatEntryTable natEntries;

    SctpNatTable();

    ~SctpNatTable();

    uint64_t& nextEntryNumber = SIMULATION_SHARED_COUNTER(nextEntryNumber);

//    void addNatEntry(SctpNatEntry* entry);

    SctpNatEntry *findNatEntry(L3Address srcAddr, uint16_t srcPrt, L3Address destAddr, uint16_t destPrt, uint32_t globalVtag);

    SctpNatEntry *getEntry(L3Address globalAddr, uint16_t globalPrt, L3Address nattedAddr, uint16_t nattedPrt, uint32_t localVtag);

    SctpNatEntry *getSpecialEntry(L3Address globalAddr, uint16_t globalPrt, L3Address nattedAddr, uint16_t nattedPrt);

    SctpNatEntry *getLocalInitEntry(L3Address globalAddr, uint16_t localPrt, uint16_t globalPrt);

    SctpNatEntry *getLocalEntry(L3Address globalAddr, uint16_t localPrt, uint16_t globalPrt, uint32_t localVtag);

    void removeEntry(SctpNatEntry *entry);

    void printNatTable();

    uint64_t getNextEntryNumber() { return nextEntryNumber++; };
};

} // namespace sctp
} // namespace inet

#endif

