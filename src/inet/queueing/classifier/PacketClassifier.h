//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PACKETCLASSIFIER_H
#define __INET_PACKETCLASSIFIER_H

#include "inet/queueing/base/PacketClassifierBase.h"
#include "inet/queueing/contract/IPacketClassifierFunction.h"

namespace inet {
namespace queueing {

class INET_API PacketClassifier : public PacketClassifierBase
{
  protected:
    IPacketClassifierFunction *packetClassifierFunction = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual IPacketClassifierFunction *createClassifierFunction(const char *classifierClass) const;
    virtual int classifyPacket(Packet *packet) override;

  public:
    virtual ~PacketClassifier() { delete packetClassifierFunction; }
};

} // namespace queueing
} // namespace inet

#endif

