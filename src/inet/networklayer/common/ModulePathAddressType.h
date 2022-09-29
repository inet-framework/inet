//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_MODULEPATHADDRESSTYPE_H
#define __INET_MODULEPATHADDRESSTYPE_H

#include "inet/networklayer/common/ModulePathAddress.h"
#include "inet/networklayer/contract/IL3AddressType.h"

namespace inet {

class INET_API ModulePathAddressType : public IL3AddressType
{
  public:
    static const ModulePathAddressType INSTANCE;

  public:
    ModulePathAddressType() {}
    virtual ~ModulePathAddressType() {}

    virtual int getAddressBitLength() const override { return 64; } // change to your choice
    virtual int getMaxPrefixLength() const override { return 0; } // TODO support address prefixes
    virtual L3Address getUnspecifiedAddress() const override { return ModulePathAddress(); }
    virtual L3Address getBroadcastAddress() const override { return ModulePathAddress(-1); }
    virtual L3Address getLinkLocalManetRoutersMulticastAddress() const override { return ModulePathAddress(-109); } // TODO constant
    virtual L3Address getLinkLocalRIPRoutersMulticastAddress() const override { return ModulePathAddress(-9); } // TODO constant
    virtual const Protocol *getNetworkProtocol() const override { return &Protocol::nextHopForwarding; }
    virtual L3Address getLinkLocalAddress(const NetworkInterface *ie) const override { return ModulePathAddress(); } // TODO constant
};

} // namespace inet

#endif

