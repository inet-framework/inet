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

#ifndef __INET_PACKETDEMULTIPLEXER_H
#define __INET_PACKETDEMULTIPLEXER_H

#include "inet/queueing/base/PacketProcessorBase.h"
#include "inet/queueing/contract/IActivePacketSink.h"
#include "inet/queueing/contract/IPassivePacketSource.h"

namespace inet {
namespace queueing {

class INET_API PacketDemultiplexer : public PacketProcessorBase, public virtual IActivePacketSink, public virtual IPassivePacketSource
{
  protected:
    cGate *inputGate = nullptr;
    IPassivePacketSource *provider = nullptr;

    std::vector<cGate *> outputGates;
    std::vector<IActivePacketSink *> collectors;

  protected:
    virtual void initialize(int stage) override;

  public:
    virtual IPassivePacketSource *getProvider(cGate *gate) override { return provider; }

    virtual bool supportsPacketPushing(cGate *gate) const override { return false; }
    virtual bool supportsPacketPulling(cGate *gate) const override { return true; }

    virtual bool canPullSomePacket(cGate *gate) const override { return provider->canPullSomePacket(inputGate->getPathStartGate()); }
    virtual Packet *canPullPacket(cGate *gate) const override { return  provider->canPullPacket(inputGate->getPathStartGate()); }

    virtual Packet* pullPacket(cGate *gate) override;

    virtual Packet *pullPacketStart(cGate *gate, bps datarate) override { throw cRuntimeError("Invalid operation"); }
    virtual Packet *pullPacketEnd(cGate *gate, bps datarate) override { throw cRuntimeError("Invalid operation"); }
    virtual Packet *pullPacketProgress(cGate *gate, bps datarate, b position, b extraProcessableLength) override { throw cRuntimeError("Invalid operation"); }
    virtual b getPullPacketProcessedLength(Packet *packet, cGate *gate) override { throw cRuntimeError("Invalid operation"); }

    virtual void handlePullPacketProcessed(Packet *packet, cGate *gate, bool successful) override;
    virtual void handleCanPullPacketChanged(cGate *gate) override;
};

} // namespace queueing
} // namespace inet

#endif // ifndef __INET_PACKETDEMULTIPLEXER_H

