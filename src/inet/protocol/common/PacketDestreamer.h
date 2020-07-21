//
// Copyright (C) OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see http://www.gnu.org/licenses/.
//

#ifndef __INET_PACKETDESTREAMER_H
#define __INET_PACKETDESTREAMER_H

#include "inet/queueing/base/PacketProcessorBase.h"
#include "inet/queueing/contract/IPacketFlow.h"

namespace inet {

using namespace inet::queueing;

class INET_API PacketDestreamer : public PacketProcessorBase, public virtual IPacketFlow
{
  protected:
    bps datarate;

    cGate *inputGate = nullptr;
    IActivePacketSource *producer = nullptr;
    IPassivePacketSource *provider = nullptr;

    cGate *outputGate = nullptr;
    IPassivePacketSink *consumer = nullptr;
    IActivePacketSink *collector = nullptr;

    bps streamDatarate = bps(NaN);
    Packet *streamedPacket = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;

    virtual bool isStreaming() const { return streamedPacket != nullptr; }

  public:
    virtual IPassivePacketSink *getConsumer(cGate *gate) override { return this; }
    virtual IPassivePacketSource *getProvider(cGate *gate) override { return this; }

    virtual bool supportsPacketPushing(cGate *gate) const override { return true; }
    virtual bool supportsPacketPulling(cGate *gate) const override { return true; }
    virtual bool supportsPacketPassing(cGate *gate) const override { return gate == outputGate; }
    virtual bool supportsPacketStreaming(cGate *gate) const override { return gate == inputGate; }

    virtual bool canPushSomePacket(cGate *gate) const override;
    virtual bool canPushPacket(Packet *packet, cGate *gate) const override;
    virtual void pushPacket(Packet *packet, cGate *gate) override { throw cRuntimeError("Invalid operation"); }

    virtual void pushPacketStart(Packet *packet, cGate *gate, bps datarate) override;
    virtual void pushPacketEnd(Packet *packet, cGate *gate, bps datarate) override;
    virtual void pushPacketProgress(Packet *packet, cGate *gate, bps datarate, b position, b extraProcessableLength = b(0)) override;
    virtual b getPushPacketProcessedLength(Packet *packet, cGate *gate) override;

    virtual void handleCanPushPacketChanged(cGate *gate) override;
    virtual void handlePushPacketProcessed(Packet *packet, cGate *gate, bool successful) override;

    virtual bool canPullSomePacket(cGate *gate) const override;
    virtual Packet *canPullPacket(cGate *gate) const override;
    virtual Packet *pullPacket(cGate *gate) override;

    virtual Packet *pullPacketStart(cGate *gate, bps datarate) override { throw cRuntimeError("Invalid operation"); }
    virtual Packet *pullPacketEnd(cGate *gate, bps datarate) override { throw cRuntimeError("Invalid operation"); }
    virtual Packet *pullPacketProgress(cGate *gate, bps datarate, b position, b extraProcessableLength) override { throw cRuntimeError("Invalid operation"); }
    virtual b getPullPacketProcessedLength(Packet *packet, cGate *gate) override { throw cRuntimeError("Invalid operation"); }

    virtual void handlePullPacketProcessed(Packet *packet, cGate *gate, bool successful) override;
    virtual void handleCanPullPacket(cGate *gate) override;
};

} // namespace inet

#endif // ifndef __INET_PACKETDESTREAMER_H

