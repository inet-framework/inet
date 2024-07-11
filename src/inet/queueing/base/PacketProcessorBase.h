//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PACKETPROCESSORBASE_H
#define __INET_PACKETPROCESSORBASE_H

#include "inet/common/packet/Packet.h"
#include "inet/common/Simsignals.h"
#include "inet/common/StringFormat.h"
#include "inet/queueing/common/PassivePacketSinkRef.h"
#include "inet/queueing/contract/IPacketProcessor.h"
#include "inet/queueing/base/AnimatePacket.h"

namespace inet {
namespace queueing {

class INET_API PacketProcessorBase : public cSimpleModule, public virtual IPacketProcessor, public StringFormat::IDirectiveResolver
{
  protected:
    const char *displayStringTextFormat = nullptr;
    int numProcessedPackets = -1;
    b processedTotalLength = b(-1);

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void refreshDisplay() const override;
    virtual void handlePacketProcessed(Packet *packet);

    virtual void checkPacketOperationSupport(cGate *gate) const;
    virtual void checkPacketOperationSupport(cGate *startGate, cGate *endGate) const;

    virtual void pushOrSendPacket(Packet *packet, cGate *startGate, PassivePacketSinkRef& consumer);
    virtual void pushOrSendPacketStart(Packet *packet, cGate *startGate, PassivePacketSinkRef& consumer, bps datarate, int transmissionId);
    virtual void pushOrSendPacketEnd(Packet *packet, cGate *startGate, PassivePacketSinkRef& consumer, int transmissionId);
    virtual void pushOrSendPacketProgress(Packet *packet, cGate *startGate, PassivePacketSinkRef &consumer, bps datarate, b position, b extraProcessableLength, int transmissionId);

    virtual void dropPacket(Packet *packet, PacketDropReason reason, int limit = -1);

    virtual void updateDisplayString() const;

  public:
    virtual bool supportsPacketSending(const cGate *gate) const override { return true; }
    virtual bool supportsPacketPassing(const cGate *gate) const override { return true; }
    virtual bool supportsPacketStreaming(const cGate *gate) const override { return false; }

    virtual std::string resolveDirective(char directive) const override;
};

} // namespace queueing
} // namespace inet

#endif

