//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE8022LLCSOCKETPACKETPROCESSOR_H
#define __INET_IEEE8022LLCSOCKETPACKETPROCESSOR_H

#include "inet/common/ModuleRefByPar.h"
#include "inet/linklayer/ieee8022/Ieee8022LlcSocketTable.h"
#include "inet/queueing/base/PacketPusherBase.h"

namespace inet {

class INET_API Ieee8022LlcSocketPacketProcessor : public queueing::PacketPusherBase
{
  protected:
    ModuleRefByPar<Ieee8022LlcSocketTable> socketTable;

  protected:
    virtual void initialize(int stage) override;

  public:
    virtual void pushPacket(Packet *packet, const cGate *gate) override;
};

} // namespace inet

#endif

