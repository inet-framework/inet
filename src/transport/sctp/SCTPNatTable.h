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

#ifndef __SCTPNATTABLE_H
#define __SCTPNATTABLE_H

#include <vector>
#include <omnetpp.h>
#include "IPvXAddress.h"
#include "SCTPAssociation.h"


class INET_API SCTPNatEntry : public cPolymorphic
{
  protected:
    uint32 entryNumber;
    IPvXAddress localAddress;
    IPvXAddress globalAddress;
    IPvXAddress nattedAddress;
    uint16 localPort;
    uint16 globalPort;
    uint16 nattedPort;
    uint32 globalVtag;
    uint32 localVtag;

  public:
    SCTPNatEntry();
    ~SCTPNatEntry();

    cMessage* NatTimer;
    void setLocalAddress(IPvXAddress addr) {localAddress = addr;};
    void setGlobalAddress(IPvXAddress addr) {globalAddress = addr;};
    void setNattedAddress(IPvXAddress addr) {nattedAddress = addr;};
    void setLocalPort(uint16 port) {localPort = port;};
    void setGlobalPort(uint16 port) {globalPort = port;};
    void setNattedPort(uint16 port) {nattedPort = port;};
    void setGlobalVTag(uint32 tag) {globalVtag = tag;};
    void setLocalVTag(uint32 tag) {localVtag = tag;};
    void setEntryNumber(uint32 number) {entryNumber = number;};

    IPvXAddress getLocalAddress() {return localAddress;};
    IPvXAddress getGlobalAddress() {return globalAddress;};
    IPvXAddress getNattedAddress() {return nattedAddress;};
    uint16 getLocalPort() {return localPort;};
    uint16 getGlobalPort() {return globalPort;};
    uint16 getNattedPort() {return nattedPort;};
    uint32 getGlobalVTag() {return globalVtag;};
    uint32 getLocalVTag() {return localVtag;};
};


class INET_API SCTPNatTable : public cSimpleModule
{
  public:

    typedef std::vector<SCTPNatEntry*> SCTPNatEntryTable;

    SCTPNatEntryTable natEntries;

    SCTPNatTable();

    ~SCTPNatTable();

    static uint32 nextEntryNumber;

    //void addNatEntry(SCTPNatEntry* entry);

    SCTPNatEntry* findNatEntry(IPvXAddress srcAddr, uint16 srcPrt, IPvXAddress destAddr, uint16 destPrt, uint32 globalVtag);

    SCTPNatEntry* getEntry(IPvXAddress globalAddr, uint16 globalPrt, IPvXAddress nattedAddr, uint16 nattedPrt, uint32 localVtag);

    SCTPNatEntry* getSpecialEntry(IPvXAddress globalAddr, uint16 globalPrt, IPvXAddress nattedAddr, uint16 nattedPrt);

    SCTPNatEntry* getLocalInitEntry(IPvXAddress globalAddr, uint16 localPrt, uint16 globalPrt);

    SCTPNatEntry* getLocalEntry(IPvXAddress globalAddr, uint16 localPrt, uint16 globalPrt, uint32 localVtag);

    void removeEntry(SCTPNatEntry* entry);

    void printNatTable();

    static uint32 getNextEntryNumber() {return nextEntryNumber++;};
};

#endif
