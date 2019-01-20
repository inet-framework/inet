//
// Copyright (C) 2013 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_IL3ADDRESSTYPE_H
#define __INET_IL3ADDRESSTYPE_H

#include "inet/common/INETDefs.h"
#include "inet/common/Protocol.h"
#include "inet/networklayer/common/InterfaceEntry.h"
#include "inet/networklayer/common/L3Address.h"

namespace inet {

/**
 * This class provides the abstract interface for different address types.
 */
class INET_API IL3AddressType
{
  public:
    IL3AddressType() {}
    virtual ~IL3AddressType() {}
    int getAddressByteLength() const { return (getAddressBitLength() + 7) / 8; };

    virtual int getAddressBitLength() const = 0;   // returns address representation length on network (bits)
    virtual int getMaxPrefixLength() const = 0;
    virtual L3Address getUnspecifiedAddress() const = 0;
    virtual L3Address getBroadcastAddress() const = 0;
    virtual L3Address getLinkLocalManetRoutersMulticastAddress() const = 0;
    virtual L3Address getLinkLocalRIPRoutersMulticastAddress() const = 0;
    virtual const Protocol *getNetworkProtocol() const = 0;    // TODO: move, where?

    /**
     * Returns the first valid link-local address of the interface, or UNSPECIFIED_ADDRESS if there's none.
     */
    virtual L3Address getLinkLocalAddress(const InterfaceEntry *ie) const = 0;
};

} // namespace inet

#endif // ifndef __INET_IL3ADDRESSTYPE_H

