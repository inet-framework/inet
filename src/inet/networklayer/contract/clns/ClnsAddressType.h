//
// Copyright (C) 2012 - 2016 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


/**
 * @file CLNSAddressType.h
 * @author Marcel Marek (mailto:imarek@fit.vutbr.cz), Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)
 * @date 10.8.2016
 * @brief Class representing a CLNS Address type
 */

#ifndef __INET_CLNSADDRESSTYPE_H
#define __INET_CLNSADDRESSTYPE_H

#include "inet/networklayer/contract/IL3AddressType.h"
#include "inet/networklayer/contract/clns/ClnsAddress.h"

namespace inet {

class INET_API CLNSAddressType : public IL3AddressType
{
  public:
    static const CLNSAddressType INSTANCE;

  public:
    CLNSAddressType() {}
    virtual ~CLNSAddressType() {}

    virtual int getAddressBitLength() const override { return 32; }
    virtual int getMaxPrefixLength() const override { return 32; }
    virtual L3Address getUnspecifiedAddress() const override { return ClnsAddress::UNSPECIFIED_ADDRESS; }
    virtual L3Address getBroadcastAddress() const override { return ClnsAddress::UNSPECIFIED_ADDRESS; }
    virtual L3Address getLinkLocalManetRoutersMulticastAddress() const override { return ClnsAddress::UNSPECIFIED_ADDRESS; }
    virtual L3Address getLinkLocalRIPRoutersMulticastAddress() const override { return ClnsAddress::UNSPECIFIED_ADDRESS; }
    virtual const Protocol *getNetworkProtocol() const override { return &Protocol::clns; }

    virtual L3Address getLinkLocalAddress(const NetworkInterface *ie) const override { return ClnsAddress::UNSPECIFIED_ADDRESS; }
};

} /* namespace inet */

#endif

