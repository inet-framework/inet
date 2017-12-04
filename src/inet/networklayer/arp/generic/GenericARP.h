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

#include "inet/common/INETDefs.h"

#include "inet/networklayer/common/L3Address.h"
#include "inet/linklayer/common/MacAddress.h"
#include "inet/common/ModuleAccess.h"
#include "inet/networklayer/common/InterfaceEntry.h"
#include "inet/networklayer/contract/IArp.h"

namespace inet {

class IInterfaceTable;

/**
 * TODO GenericMACAddressCache
 *
 * - translates L3 address to L2 addresses
 * - uses global mapping (or table, or algorithm)
 * - not a protocol -- doesn't communicate
 */
class INET_API GenericArp : public cSimpleModule, public IArp
{
  private:
    IInterfaceTable *ift = nullptr;

  public:
    GenericArp() {}
    virtual ~GenericArp() {}
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }

    virtual MacAddress resolveL3Address(const L3Address& address, const InterfaceEntry *ie) override;
    virtual L3Address getL3AddressFor(const MacAddress&) const override { throw cRuntimeError("getL3AddressFor() not implemented yet"); }

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;

    MacAddress mapUnicastAddress(L3Address addr);
    MacAddress mapMulticastAddress(L3Address addr);
    void sendPacketToNIC(cMessage *msg, InterfaceEntry *ie, const MacAddress& macAddress, int etherType);
};

} // namespace inet

#endif // ifndef __INET_GENERICARP_H

