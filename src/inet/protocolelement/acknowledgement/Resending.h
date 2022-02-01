//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_RESENDING_H
#define __INET_RESENDING_H

#include "inet/queueing/base/PacketPusherBase.h"

namespace inet {

using namespace inet::queueing;

class INET_API Resending : public PacketPusherBase
{
  protected:
    int numRetries = 0;
    queueing::IActivePacketSource *producer = nullptr;

    Packet *packet = nullptr;
    int retry = 0;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;

  public:
    virtual ~Resending() { delete packet; }
    virtual void handlePushPacketProcessed(Packet *packet, cGate *gate, bool successful) override;

    virtual bool canPushSomePacket(cGate *gate) const override { return packet == nullptr; }
    virtual bool canPushPacket(Packet *packet, cGate *gate) const override { return packet == nullptr; }
    virtual void pushPacket(Packet *packet, cGate *gate) override;
};

} // namespace inet

#endif

