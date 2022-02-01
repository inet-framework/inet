//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PRIORITYCLASSIFIER_H
#define __INET_PRIORITYCLASSIFIER_H

#include "inet/queueing/base/PacketClassifierBase.h"
#include "inet/queueing/contract/IPacketCollection.h"

namespace inet {
namespace queueing {

class INET_API PriorityClassifier : public PacketClassifierBase
{
  protected:
    virtual int classifyPacket(Packet *packet) override;

  public:
    virtual bool canPushSomePacket(cGate *gate) const override;
    virtual bool canPushPacket(Packet *packet, cGate *gate) const override;
};

} // namespace queueing
} // namespace inet

#endif

