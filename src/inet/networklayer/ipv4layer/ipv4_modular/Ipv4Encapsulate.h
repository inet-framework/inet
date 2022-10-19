//
// Copyright (C) 2022 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IPV4ENCAPSULATE_H
#define __INET_IPV4ENCAPSULATE_H

#include "inet/common/ModuleRefByPar.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/ipv4layer/routingtable/IIpv4RoutingTable.h"
#include "inet/queueing/base/PacketPusherBase.h"
#include "inet/transportlayer/common/CrcMode_m.h"

namespace inet {

class INET_API Ipv4Encapsulate : public queueing::PacketPusherBase
{
  protected:
    ModuleRefByPar<IInterfaceTable> ift;
    ModuleRefByPar<IIpv4RoutingTable> rt;
    CrcMode crcMode = CRC_MODE_UNDEFINED;
    int defaultTimeToLive = -1;
    int defaultMCTimeToLive = -1;
    uint16_t curFragmentId = -1; // counter, used to assign unique fragmentIds to datagrams

  protected:
    virtual void initialize(int stage) override;
    virtual void pushPacket(Packet *packet, cGate *gate) override;

    void encapsulate(Packet *transportPacket);
};

} // namespace inet

#endif
