//
// Copyright (C) 2022 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IPV4FRAMERECEIVER_H
#define __INET_IPV4FRAMERECEIVER_H

#include "inet/common/ModuleRefByPar.h"
#include "inet/networklayer/ipv4layer/icmp/Icmp.h"
#include "inet/queueing/base/PacketPusherBase.h"

namespace inet {

class INET_API Ipv4HeaderChecker : public queueing::PacketPusherBase
{
  protected:
    ModuleRefByPar<Icmp> icmp;

  protected:
    virtual void initialize(int stage) override;
    virtual void pushPacket(Packet *packet, cGate *gate) override;

    void sendIcmpError(Packet *packet, IcmpType type, IcmpCode code);
};

} // namespace inet

#endif
