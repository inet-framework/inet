//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ACCESSORYPROTOCOL_H
#define __INET_ACCESSORYPROTOCOL_H

#include "inet/common/Protocol.h"
#include "inet/common/packet/Packet.h"
#include "inet/queueing/contract/IPacketProcessor.h"

namespace inet {

class INET_API AccessoryProtocol
{
  public:
    static const Protocol acknowledge;
    static const Protocol aggregation;
    static const Protocol crc;
    static const Protocol destinationL3Address;
    static const Protocol destinationMacAddress;
    static const Protocol destinationPort;
    static const Protocol fcs;
    static const Protocol forwarding;
    static const Protocol fragmentation;
    static const Protocol hopLimit;
    static const Protocol sequenceNumber;
    static const Protocol withAcknowledge;
};

} // namespace inet

#endif

