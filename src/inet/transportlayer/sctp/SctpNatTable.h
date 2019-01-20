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
    uint32 entryNumber;
    L3Address localAddress;
    L3Address globalAddress;
    L3Address nattedAddress;
    uint16 localPort;
    uint16 globalPort;
    uint16 nattedPort;
    uint32 globalVtag;
    uint32 localVtag;

  public:
    SctpNatEntry();
    ~SctpNatEntry();

    void setLocalAddress(L3Address addr) { localAddress = addr; };
    void setGlobalAddress(L3Address addr) { globalAddress = addr; };
    void setNattedAddress(L3Address addr) { nattedAddress = addr; };
    void setLocalPort(uint16 port) { localPort = port; };
    void setGlobalPort(uint16 port) { globalPort = port; };
    void setNattedPort(uint16 port) { nattedPort = port; };
    void setGlobalVTag(uint32 tag) { globalVtag = tag; };
    void setLocalVTag(uint32 tag) { localVtag = tag; };
    void setEntryNumber(uint32 number) { entryNumber = number; };

    L3Address getLocalAddress() { return localAddress; };
    L3Address getGlobalAddress() { return globalAddress; };
    L3Address getNattedAddress() { return nattedAddress; };
    uint16 getLocalPort() { return localPort; };
    uint16 getGlobalPort() { return globalPort; };
    uint16 getNattedPort() { return nattedPort; };
    uint32 getGlobalVTag() { return globalVtag; };
    uint32 getLocalVTag() { return localVtag; };
};

class INET_API SctpNatTable : public cSimpleModule
{
  public:

    typedef std::vector<SctpNatEntry *> SctpNatEntryTable;

    SctpNatEntryTable natEntries;

    SctpNatTable();

    ~SctpNatTable();

    static uint32 nextEntryNumber;

    //void addNatEntry(SctpNatEntry* entry);

    SctpNatEntry *findNatEntry(L3Address srcAddr, uint16 srcPrt, L3Address destAddr, uint16 destPrt, uint32 globalVtag);

    SctpNatEntry *getEntry(L3Address globalAddr, uint16 globalPrt, L3Address nattedAddr, uint16 nattedPrt, uint32 localVtag);

    SctpNatEntry *getSpecialEntry(L3Address globalAddr, uint16 globalPrt, L3Address nattedAddr, uint16 nattedPrt);

    SctpNatEntry *getLocalInitEntry(L3Address globalAddr, uint16 localPrt, uint16 globalPrt);

    SctpNatEntry *getLocalEntry(L3Address globalAddr, uint16 localPrt, uint16 globalPrt, uint32 localVtag);

    void removeEntry(SctpNatEntry *entry);

    void printNatTable();

    static uint32 getNextEntryNumber() { return nextEntryNumber++; };
};

} // namespace sctp
} // namespace inet

#endif // ifndef __INET_SCTPNATTABLE_H

