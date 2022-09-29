//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_MACADDRESSTYPE_H
#define __INET_MACADDRESSTYPE_H

#include "inet/linklayer/common/MacAddress.h"
#include "inet/networklayer/contract/IL3AddressType.h"

namespace inet {

class INET_API MacAddressType : public IL3AddressType
{
  public:
    static const MacAddressType INSTANCE;

  public:
    MacAddressType() {}
    virtual ~MacAddressType() {}

    virtual int getAddressBitLength() const override { return 48; }
    virtual int getMaxPrefixLength() const override { return 0; }
    virtual L3Address getUnspecifiedAddress() const override { return MacAddress::UNSPECIFIED_ADDRESS; }
    virtual L3Address getBroadcastAddress() const override { return MacAddress::BROADCAST_ADDRESS; }
    virtual L3Address getLinkLocalManetRoutersMulticastAddress() const override { return MacAddress(-109); } // TODO constant
    virtual L3Address getLinkLocalRIPRoutersMulticastAddress() const override { return MacAddress(-9); } // TODO constant
    virtual const Protocol *getNetworkProtocol() const override { throw cRuntimeError("address is MacAddress, unknown L3 protocol"); }
    virtual L3Address getLinkLocalAddress(const NetworkInterface *ie) const override { return MacAddress::UNSPECIFIED_ADDRESS; }
};

} // namespace inet

#endif

