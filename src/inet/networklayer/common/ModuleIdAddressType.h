//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_MODULEIDADDRESSTYPE_H
#define __INET_MODULEIDADDRESSTYPE_H

#include "inet/networklayer/common/ModuleIdAddress.h"
#include "inet/networklayer/contract/IL3AddressType.h"

namespace inet {

class INET_API ModuleIdAddressType : public IL3AddressType
{
  public:
    static const ModuleIdAddressType INSTANCE;

  public:
    ModuleIdAddressType() {}
    virtual ~ModuleIdAddressType() {}

    virtual int getAddressBitLength() const override { return 64; } // change to your choice
    virtual int getMaxPrefixLength() const override { return 0; }
    virtual L3Address getUnspecifiedAddress() const override { return ModuleIdAddress(); } // TODO constant
    virtual L3Address getBroadcastAddress() const override { return ModuleIdAddress(-1); }
    virtual L3Address getLinkLocalManetRoutersMulticastAddress() const override { return ModuleIdAddress(-109); } // TODO constant
    virtual L3Address getLinkLocalRIPRoutersMulticastAddress() const override { return ModuleIdAddress(-9); } // TODO constant
    virtual const Protocol *getNetworkProtocol() const override { return &Protocol::nextHopForwarding; }
    virtual L3Address getLinkLocalAddress(const NetworkInterface *ie) const override { return ModuleIdAddress(); } // TODO constant
};

} // namespace inet

#endif

