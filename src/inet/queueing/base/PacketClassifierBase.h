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

#ifndef __INET_PACKETCLASSIFIERBASE_H
#define __INET_PACKETCLASSIFIERBASE_H

#include "inet/queueing/base/PassivePacketSinkBase.h"
#include "inet/queueing/contract/IActivePacketSource.h"

namespace inet {
namespace queueing {

class INET_API PacketClassifierBase : public PassivePacketSinkBase, public IActivePacketSource
{
  protected:
    cGate *inputGate = nullptr;
    IActivePacketSource *producer = nullptr;

    std::vector<cGate *> outputGates;
    std::vector<IPassivePacketSink *> consumers;

  protected:
    virtual void initialize(int stage) override;
    virtual int classifyPacket(Packet *packet) = 0;

  public:
    virtual IPassivePacketSink *getConsumer(cGate *gate) override { return consumers[gate->getIndex()]; }

    virtual bool supportsPushPacket(cGate *gate) const override { return true; }
    virtual bool supportsPopPacket(cGate *gate) const override { return false; }

    virtual bool canPushSomePacket(cGate *gate) const override;
    virtual bool canPushPacket(Packet *packet, cGate *gate) const override { return canPushSomePacket(gate); }
    virtual void pushPacket(Packet *packet, cGate *gate) override;

    virtual void handleCanPushPacket(cGate *gate) override;
};

} // namespace queueing
} // namespace inet

#endif // ifndef __INET_PACKETCLASSIFIERBASE_H

