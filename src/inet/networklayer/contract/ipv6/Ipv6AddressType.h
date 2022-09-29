//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IPV6ADDRESSTYPE_H
#define __INET_IPV6ADDRESSTYPE_H

#include "inet/common/Protocol.h"
#include "inet/networklayer/contract/IL3AddressType.h"
#include "inet/networklayer/contract/ipv6/Ipv6Address.h"

namespace inet {

class INET_API Ipv6AddressType : public IL3AddressType
{
  public:
    static const Ipv6AddressType INSTANCE;
    static const Ipv6Address ALL_RIP_ROUTERS_MCAST;

  public:
    Ipv6AddressType() {}
    virtual ~Ipv6AddressType() {}

    virtual int getAddressBitLength() const override { return 128; }
    virtual int getMaxPrefixLength() const override { return 128; }
    virtual L3Address getUnspecifiedAddress() const override { return Ipv6Address::UNSPECIFIED_ADDRESS; }
    virtual L3Address getBroadcastAddress() const override { return Ipv6Address::ALL_NODES_1; }
    virtual L3Address getLinkLocalManetRoutersMulticastAddress() const override { return Ipv6Address::LL_MANET_ROUTERS; }
    virtual L3Address getLinkLocalRIPRoutersMulticastAddress() const override { return ALL_RIP_ROUTERS_MCAST; }
    virtual const Protocol *getNetworkProtocol() const override { return &Protocol::ipv6; }
    virtual L3Address getLinkLocalAddress(const NetworkInterface *ie) const override;
};

} // namespace inet

#endif

