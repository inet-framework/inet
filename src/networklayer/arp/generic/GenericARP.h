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

#ifndef __INET_GENERICARP_H
#define __INET_GENERICARP_H

#include "INETDefs.h"

#include "Address.h"
#include "MACAddress.h"
#include "ModuleAccess.h"
#include "InterfaceEntry.h"
#include "IARP.h"

namespace inet {
class IInterfaceTable;

/**
 * TODO GenericMACAddressCache
 *
 * - translates L3 address to L2 addresses
 * - uses global mapping (or table, or algorithm)
 * - not a protocol -- doesn't communicate
 */
class INET_API GenericARP : public cSimpleModule, public IARP
{
  private:
    IInterfaceTable *ift;

  public:
    GenericARP() {}
    virtual ~GenericARP() {}
    virtual int numInitStages() const { return NUM_INIT_STAGES; }

    virtual MACAddress resolveL3Address(const Address& address, const InterfaceEntry *ie);
    virtual Address getL3AddressFor(const MACAddress&) const { throw cRuntimeError("getL3AddressFor() not implemented yet"); }

  protected:
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg);

    MACAddress mapUnicastAddress(Address addr);
    MACAddress mapMulticastAddress(Address addr);
    void sendPacketToNIC(cMessage *msg, InterfaceEntry *ie, const MACAddress& macAddress, int etherType);
};
} // namespace inet

#endif // ifndef __INET_GENERICARP_H

