//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PCAPFILEPACKETCONSUMER_H
#define __INET_PCAPFILEPACKETCONSUMER_H

#include "inet/common/packet/recorder/PcapWriter.h"
#include "inet/queueing/base/PassivePacketSinkBase.h"
#include "inet/queueing/contract/IActivePacketSource.h"

namespace inet {
namespace queueing {

class INET_API PcapFilePacketConsumer : public PassivePacketSinkBase
{
  protected:
    PcapWriter pcapWriter;
    Direction direction = DIRECTION_UNDEFINED;
    PcapLinkType networkType = LINKTYPE_INVALID;

  protected:
    virtual void initialize(int stage) override;
    virtual void finish() override;

  public:
    virtual bool supportsPacketPushing(cGate *gate) const override { return gate == inputGate; }
    virtual bool supportsPacketPulling(cGate *gate) const override { return false; }

    virtual void pushPacket(Packet *packet, cGate *gate) override;
};

} // namespace queueing
} // namespace inet

#endif

