//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PCAPFILEPACKETCONSUMER_H
#define __INET_PCAPFILEPACKETCONSUMER_H

#include "inet/common/packet/recorder/IPcapWriter.h"
#include "inet/queueing/base/PassivePacketSinkBase.h"
#include "inet/queueing/contract/IActivePacketSource.h"

namespace inet {
namespace queueing {

class INET_API PcapFilePacketConsumer : public PassivePacketSinkBase
{
  protected:
    IPcapWriter *pcapWriter = nullptr;
    Direction direction = DIRECTION_UNDEFINED;
    PcapLinkType networkType = LINKTYPE_INVALID;

  protected:
    virtual void initialize(int stage) override;
    virtual void finish() override;

  public:
    virtual bool supportsPacketPushing(const cGate *gate) const override { return gate == inputGate; }
    virtual bool supportsPacketPulling(const cGate *gate) const override { return false; }

    virtual void pushPacket(Packet *packet, const cGate *gate) override;
};

} // namespace queueing
} // namespace inet

#endif

