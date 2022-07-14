//
// Copyright (C) 2022 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IPV4DECAPSULATE_H
#define __INET_IPV4DECAPSULATE_H

#include "inet/common/ModuleRefByPar.h"
#include "inet/common/Protocol.h"
#include "inet/common/stlutils.h"
#include "inet/common/socket/SocketTag_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/DscpTag_m.h"
#include "inet/networklayer/common/EcnTag_m.h"
#include "inet/networklayer/common/HopLimitTag_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/common/TosTag_m.h"
#include "inet/networklayer/contract/ipv4/Ipv4Address.h"
#include "inet/networklayer/contract/ipv4/Ipv4SocketCommand_m.h"
#include "inet/networklayer/ipv4/Icmp.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#include "inet/queueing/base/PacketPusherBase.h"

namespace inet {

// reassembleAndDeliverFinish
class INET_API Ipv4Decapsulate : public queueing::PacketPusherBase
{
  protected:
    virtual void pushPacket(Packet *packet, cGate *gate) override;
    void decapsulate(Packet *packet);
};

} // namespace inet

#endif
