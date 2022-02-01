//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IL3ADDRESSTYPE_H
#define __INET_IL3ADDRESSTYPE_H

#include "inet/common/Protocol.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/networklayer/common/NetworkInterface.h"

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

    virtual int getAddressBitLength() const = 0; // returns address representation length on network (bits)
    virtual int getMaxPrefixLength() const = 0;
    virtual L3Address getUnspecifiedAddress() const = 0;
    virtual L3Address getBroadcastAddress() const = 0;
    virtual L3Address getLinkLocalManetRoutersMulticastAddress() const = 0;
    virtual L3Address getLinkLocalRIPRoutersMulticastAddress() const = 0;
    virtual const Protocol *getNetworkProtocol() const = 0; // TODO move, where?

    /**
     * Returns the first valid link-local address of the interface, or UNSPECIFIED_ADDRESS if there's none.
     */
    virtual L3Address getLinkLocalAddress(const NetworkInterface *ie) const = 0;
};

} // namespace inet

#endif

