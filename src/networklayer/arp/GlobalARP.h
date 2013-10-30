/*
 * Copyright (C) 2004 Andras Varga
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __INET_GLOBAL_ARP_H
#define __INET_GLOBAL_ARP_H

#include "INETDefs.h"

#include "Address.h"
#include "MACAddress.h"
#include "ModuleAccess.h"
#include "InterfaceEntry.h"

class IInterfaceTable;

/**
 * TODO GlobalMACAddressCache
 *
 * - translates L3 address to L2 addresses
 * - uses global mapping (or table, or algorithm)
 * - not a protocol -- doesn't communicate
 */
class INET_API GlobalARP : public InetSimpleModule
{
  private:
    IInterfaceTable *ift;
    int nicOutBaseGateId;  // id of the nicOut[0] gate

  public:
    GlobalARP() { }
    virtual ~GlobalARP() { }
    virtual int numInitStages() const { return NUM_INIT_STAGES; }

  protected:
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg);

    MACAddress mapUnicastAddress(Address addr);
    MACAddress mapMulticastAddress(Address addr);
    void sendPacketToNIC(cMessage *msg, InterfaceEntry *ie, const MACAddress& macAddress, int etherType);
};

#endif
