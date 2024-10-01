//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ETHERNETSOCKETPACKETPROCESSOR_H
#define __INET_ETHERNETSOCKETPACKETPROCESSOR_H

#include "inet/common/ModuleRefByPar.h"
#include "inet/linklayer/ethernet/modular/EthernetSocketTable.h"
#include "inet/queueing/base/PacketPusherBase.h"

namespace inet {

class INET_API EthernetSocketPacketProcessor : public queueing::PacketPusherBase
{
  protected:
    ModuleRefByPar<EthernetSocketTable> socketTable;

  protected:
    virtual void initialize(int stage) override;

  public:
    virtual void pushPacket(Packet *packet, const cGate *gate) override;
};

} // namespace inet

#endif

