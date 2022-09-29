//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IPV4ADDRESSTYPE_H
#define __INET_IPV4ADDRESSTYPE_H

#include "inet/common/Protocol.h"
#include "inet/networklayer/contract/IL3AddressType.h"
#include "inet/networklayer/contract/ipv4/Ipv4Address.h"

namespace inet {

class INET_API Ipv4AddressType : public IL3AddressType
{
  public:
    static const Ipv4AddressType INSTANCE;
    static const Ipv4Address ALL_RIP_ROUTERS_MCAST;

  public:
    Ipv4AddressType() {}
    virtual ~Ipv4AddressType() {}

    virtual int getAddressBitLength() const override { return 32; }
    virtual int getMaxPrefixLength() const override { return 32; }
    virtual L3Address getUnspecifiedAddress() const override { return Ipv4Address::UNSPECIFIED_ADDRESS; }
    virtual L3Address getBroadcastAddress() const override { return Ipv4Address::ALLONES_ADDRESS; }
    virtual L3Address getLinkLocalManetRoutersMulticastAddress() const override { return Ipv4Address::LL_MANET_ROUTERS; }
    virtual L3Address getLinkLocalRIPRoutersMulticastAddress() const override { return ALL_RIP_ROUTERS_MCAST; }
    virtual const Protocol *getNetworkProtocol() const override { return &Protocol::ipv4; }

    virtual L3Address getLinkLocalAddress(const NetworkInterface *ie) const override { return Ipv4Address::UNSPECIFIED_ADDRESS; }
};

} // namespace inet

#endif

