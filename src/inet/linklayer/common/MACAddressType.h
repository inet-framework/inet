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
#include "inet/networklayer/common/IL3AddressType.h"
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

    virtual int getMaxPrefixLength() const { return 0; }
    virtual L3Address getUnspecifiedAddress() const { return MACAddress::UNSPECIFIED_ADDRESS; }
    virtual L3Address getBroadcastAddress() const { return MACAddress::BROADCAST_ADDRESS; }
    virtual L3Address getLinkLocalManetRoutersMulticastAddress() const { return MACAddress(-109); }    // TODO: constant
    virtual L3Address getLinkLocalRIPRoutersMulticastAddress() const { return MACAddress(-9); }    // TODO: constant
    virtual INetworkProtocolControlInfo *createNetworkProtocolControlInfo() const { return new GenericNetworkProtocolControlInfo(); }
    virtual L3Address getLinkLocalAddress(const InterfaceEntry *ie) const { return MACAddress::UNSPECIFIED_ADDRESS; }
};

} // namespace inet

#endif // ifndef __INET_MACADDRESSTYPE_H

