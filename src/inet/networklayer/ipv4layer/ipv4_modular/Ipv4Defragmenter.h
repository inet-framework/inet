//
// Copyright (C) 2022 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IPV4DEFRAGMENTER_H
#define __INET_IPV4DEFRAGMENTER_H

#include "inet/common/ModuleRefByPar.h"
#include "inet/networklayer/ipv4layer/common/Ipv4FragBuf.h"
#include "inet/networklayer/ipv4layer/icmp/Icmp.h"
#include "inet/queueing/base/PacketPusherBase.h"

namespace inet {

class INET_API Ipv4Defragmenter : public queueing::PacketPusherBase
{
  protected:
    ModuleRefByPar<Icmp> icmp;
    Ipv4FragBuf fragbuf; // fragmentation reassembly buffer
    simtime_t lastCheckTime; // when fragbuf was last checked for state fragments
    simtime_t fragmentTimeoutTime;

  protected:
    virtual void initialize(int stage) override;
    virtual void pushPacket(Packet *packet, cGate *gate) override;

    Packet *reassembleAndDeliver(Packet *packet);
};

} // namespace inet

#endif
