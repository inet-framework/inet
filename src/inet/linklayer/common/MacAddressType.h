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

#ifndef __INET_MACADDRESSTYPE_H
#define __INET_MACADDRESSTYPE_H

#include "inet/common/INETDefs.h"
#include "inet/linklayer/common/MacAddress.h"
#include "inet/networklayer/contract/IL3AddressType.h"

namespace inet {

class INET_API MacAddressType : public IL3AddressType
{
  public:
    static MacAddressType INSTANCE;

  public:
    MacAddressType() {}
    virtual ~MacAddressType() {}

    virtual int getAddressBitLength() const override { return 48; }
    virtual int getMaxPrefixLength() const override { return 0; }
    virtual L3Address getUnspecifiedAddress() const override { return MacAddress::UNSPECIFIED_ADDRESS; }
    virtual L3Address getBroadcastAddress() const override { return MacAddress::BROADCAST_ADDRESS; }
    virtual L3Address getLinkLocalManetRoutersMulticastAddress() const override { return MacAddress(-109); }    // TODO: constant
    virtual L3Address getLinkLocalRIPRoutersMulticastAddress() const override { return MacAddress(-9); }    // TODO: constant
    virtual const Protocol *getNetworkProtocol() const override { throw cRuntimeError("address is MacAddress, unknown L3 protocol"); }
    virtual L3Address getLinkLocalAddress(const InterfaceEntry *ie) const override { return MacAddress::UNSPECIFIED_ADDRESS; }
};

} // namespace inet

#endif // ifndef __INET_MACADDRESSTYPE_H

