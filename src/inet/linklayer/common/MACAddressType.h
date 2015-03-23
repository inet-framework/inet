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
#include "inet/networklayer/contract/IL3AddressType.h"
#include "inet/linklayer/common/MACAddress.h"
#include "inet/networklayer/contract/generic/GenericNetworkProtocolControlInfo.h"

namespace inet {

class INET_API MACAddressType : public IL3AddressType
{
  public:
    static MACAddressType INSTANCE;

  public:
    MACAddressType() {}
    virtual ~MACAddressType() {}

    virtual int getAddressBitLength() const override { return 48; }
    virtual int getMaxPrefixLength() const override { return 0; }
    virtual L3Address getUnspecifiedAddress() const override { return MACAddress::UNSPECIFIED_ADDRESS; }
    virtual L3Address getBroadcastAddress() const override { return MACAddress::BROADCAST_ADDRESS; }
    virtual L3Address getLinkLocalManetRoutersMulticastAddress() const override { return MACAddress(-109); }    // TODO: constant
    virtual L3Address getLinkLocalRIPRoutersMulticastAddress() const override { return MACAddress(-9); }    // TODO: constant
    virtual INetworkProtocolControlInfo *createNetworkProtocolControlInfo() const override { return new GenericNetworkProtocolControlInfo(); }
    virtual L3Address getLinkLocalAddress(const InterfaceEntry *ie) const override { return MACAddress::UNSPECIFIED_ADDRESS; }
};

} // namespace inet

#endif // ifndef __INET_MACADDRESSTYPE_H

