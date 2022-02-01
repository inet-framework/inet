//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PCAPFILEPACKETPRODUCER_H
#define __INET_PCAPFILEPACKETPRODUCER_H

#include "inet/common/packet/recorder/PcapReader.h"
#include "inet/queueing/base/ActivePacketSourceBase.h"

namespace inet {
namespace queueing {

class INET_API PcapFilePacketProducer : public ActivePacketSourceBase
{
  protected:
    PcapReader pcapReader;

  protected:
    virtual void initialize(int stage) override;
    virtual void finish() override;
    virtual void handleMessage(cMessage *message) override;

    virtual void schedulePacket();

  public:
    virtual void handleCanPushPacketChanged(cGate *gate) override;
    virtual void handlePushPacketProcessed(Packet *packet, cGate *gate, bool successful) override {}
};

} // namespace queueing
} // namespace inet

#endif

