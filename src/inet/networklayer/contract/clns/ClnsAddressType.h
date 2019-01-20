// Copyright (C) 2012 - 2016 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

/**
 * @file CLNSAddressType.h
 * @author Marcel Marek (mailto:imarek@fit.vutbr.cz), Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)
 * @date 10.8.2016
 * @brief Class representing a CLNS Address type
  */


#ifndef ANSA_NETWORKLAYER_CLNS_CLNSADDRESSTYPE_H_
#define ANSA_NETWORKLAYER_CLNS_CLNSADDRESSTYPE_H_

#include "inet/common/INETDefs.h"
#include "inet/networklayer/contract/IL3AddressType.h"
#include "inet/networklayer/contract/clns/ClnsAddress.h"

namespace inet {

class INET_API CLNSAddressType : public IL3AddressType
{
  public:
    static CLNSAddressType INSTANCE;


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

    virtual L3Address getLinkLocalAddress(const InterfaceEntry *ie) const override { return ClnsAddress::UNSPECIFIED_ADDRESS; }
};

} /* namespace inet */

#endif /* ANSA_NETWORKLAYER_CLNS_CLNSADDRESSTYPE_H_ */
