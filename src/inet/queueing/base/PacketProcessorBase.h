//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PACKETPROCESSORBASE_H
#define __INET_PACKETPROCESSORBASE_H

#include "inet/common/packet/Packet.h"
#include "inet/common/Simsignals.h"
#include "inet/common/SimpleModule.h"
#include "inet/queueing/common/PassivePacketSinkRef.h"
#include "inet/queueing/contract/IPacketProcessor.h"

namespace inet {
namespace queueing {

class INET_API PacketProcessorBase : public SimpleModule, public virtual IPacketProcessor
{
  protected:
    enum Action {
        PUSH,
        PULL,
    };

  protected:
    const char *displayStringTextFormat = nullptr;
    int numProcessedPackets = -1;
    b processedTotalLength = b(-1);

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handlePacketProcessed(Packet *packet);

    virtual void checkPacketOperationSupport(cGate *gate) const;
    virtual void checkPacketOperationSupport(cGate *startGate, cGate *endGate) const;

    virtual void animate(Packet *packet, cGate *startGate, cGate *endGate, const SendOptions& sendOptions, Action action) const;
    virtual void animatePacket(Packet *packet, cGate *startGate, cGate *endGate, Action action) const;
    virtual void animatePacketStart(Packet *packet, cGate *startGate, cGate *endGate, bps datarate, long transmissionId, Action action) const;
    virtual void animatePacketStart(Packet *packet, cGate *startGate, cGate *endGate, bps datarate, const SendOptions& sendOptions, Action action) const;
    virtual void animatePacketEnd(Packet *packet, cGate *startGate, cGate *endGate, long transmissionId, Action action) const;
    virtual void animatePacketEnd(Packet *packet, cGate *startGate, cGate *endGate, const SendOptions& sendOptions, Action action) const;
    virtual void animatePacketProgress(Packet *packet, cGate *startGate, cGate *endGate, bps datarate, b position, b extraProcessableLength, long transmissionId, Action action) const;
    virtual void animatePacketProgress(Packet *packet, cGate *startGate, cGate *endGate, bps datarate, b position, b extraProcessableLength, const SendOptions& sendOptions, Action action) const;

    virtual void pushOrSendPacket(Packet *packet, cGate *startGate, PassivePacketSinkRef& consumer);
    virtual void pushOrSendPacketStart(Packet *packet, cGate *startGate, PassivePacketSinkRef& consumer, bps datarate, int transmissionId);
    virtual void pushOrSendPacketEnd(Packet *packet, cGate *startGate, PassivePacketSinkRef& consumer, int transmissionId);
    virtual void pushOrSendPacketProgress(Packet *packet, cGate *startGate, PassivePacketSinkRef &consumer, bps datarate, b position, b extraProcessableLength, int transmissionId);

    virtual void animatePush(Packet *packet, cGate *startGate, cGate *endGate, const SendOptions& sendOptions) const;
    virtual void animatePushPacket(Packet *packet, cGate *startGate, cGate *endGate) const;
    virtual void animatePushPacketStart(Packet *packet, cGate *startGate, cGate *endGate, bps datarate, long transmissionId) const;
    virtual void animatePushPacketStart(Packet *packet, cGate *startGate, cGate *endGate, bps datarate, const SendOptions& sendOptions) const;
    virtual void animatePushPacketEnd(Packet *packet, cGate *startGate, cGate *endGate, long transmissionId) const;
    virtual void animatePushPacketEnd(Packet *packet, cGate *startGate, cGate *endGate, const SendOptions& sendOptions) const;
    virtual void animatePushPacketProgress(Packet *packet, cGate *startGate, cGate *endGate, bps datarate, b position, b extraProcessableLength, long transmissionId) const;
    virtual void animatePushPacketProgress(Packet *packet, cGate *startGate, cGate *endGate, bps datarate, b position, b extraProcessableLength, const SendOptions& sendOptions) const;

    virtual void animatePull(Packet *packet, cGate *startGate, cGate *endGate, const SendOptions& sendOptions) const;
    virtual void animatePullPacket(Packet *packet, cGate *startGate, cGate *endGate) const;
    virtual void animatePullPacketStart(Packet *packet, cGate *startGate, cGate *endGate, bps datarate, long transmissionId) const;
    virtual void animatePullPacketStart(Packet *packet, cGate *startGate, cGate *endGate, bps datarate, const SendOptions& sendOptions) const;
    virtual void animatePullPacketEnd(Packet *packet, cGate *startGate, cGate *endGate, long transmissionId) const;
    virtual void animatePullPacketEnd(Packet *packet, cGate *startGate, cGate *endGate, const SendOptions& sendOptions) const;
    virtual void animatePullPacketProgress(Packet *packet, cGate *startGate, cGate *endGate, bps datarate, b position, b extraProcessableLength, long transmissionId) const;
    virtual void animatePullPacketProgress(Packet *packet, cGate *startGate, cGate *endGate, bps datarate, b position, b extraProcessableLength, const SendOptions& sendOptions) const;

    virtual void dropPacket(Packet *packet, PacketDropReason reason, int limit = -1);

  public:
    virtual bool supportsPacketSending(const cGate *gate) const override { return true; }
    virtual bool supportsPacketPassing(const cGate *gate) const override { return true; }
    virtual bool supportsPacketStreaming(const cGate *gate) const override { return false; }

    virtual std::string resolveDirective(char directive) const override;
};

} // namespace queueing
} // namespace inet

#endif

