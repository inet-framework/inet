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

#ifndef __INET_PACKETMETERBASE_H
#define __INET_PACKETMETERBASE_H

#include "inet/queueing/base/PacketProcessorBase.h"
#include "inet/queueing/contract/IPassivePacketSource.h"
#include "inet/queueing/contract/IActivePacketSource.h"

namespace inet {

using namespace inet::queueing;

class INET_API PacketMeterBase : public PacketProcessorBase, public IPassivePacketSink, public IActivePacketSource
{
  protected:
    cGate *inputGate = nullptr;
    IActivePacketSource *producer = nullptr;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;
    virtual int meterPacket(Packet *packet) = 0;

  public:
    virtual IPassivePacketSink *getConsumer(cGate *gate) override { throw cRuntimeError("Invalid operation"); }

    virtual bool supportsPacketPushing(cGate *gate) const override { return true; }
    virtual bool supportsPacketPulling(cGate *gate) const override { return false; }

    virtual bool canPushSomePacket(cGate *gate) const override { return true; }
    virtual bool canPushPacket(Packet *packet, cGate *gate) const override { return true; }

    virtual void handleCanPushPacket(cGate *gate) override;
    virtual void handlePushPacketProcessed(Packet *packet, cGate *gate, bool successful) override;

    virtual void pushPacketStart(Packet *packet, cGate *gate) override { throw cRuntimeError("Invalid operation"); }
    virtual void pushPacketEnd(Packet *packet, cGate *gate) override { throw cRuntimeError("Invalid operation"); }
    virtual void pushPacketProgress(Packet *packet, cGate *gate, b position, b extraProcessableLength = b(0)) override { throw cRuntimeError("Invalid operation"); }

    virtual b getPushPacketProcessedLength(Packet *packet, cGate *gate) override { throw cRuntimeError("Invalid operation"); }
};

} // namespace inet

#endif // ifndef __INET_PACKETMETERBASE_H

