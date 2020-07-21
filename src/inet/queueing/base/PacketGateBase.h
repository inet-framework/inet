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

#ifndef __INET_PACKETGATEBASE_H
#define __INET_PACKETGATEBASE_H

#include "inet/queueing/base/PacketFlowBase.h"
#include "inet/queueing/contract/IPacketGate.h"

namespace inet {
namespace queueing {

class INET_API PacketGateBase : public PacketFlowBase, public virtual IPacketGate
{
  public:
    static simsignal_t gateStateChangedSignal;

  protected:
    bool isOpen_ = false;

  protected:
    virtual void initialize(int stage) override;
    virtual void processPacket(Packet *packet) override;

  public:
    virtual bool isOpen() const override { return isOpen_; }
    virtual void open() override;
    virtual void close() override;

    virtual IPassivePacketSink *getConsumer(cGate *gate) override { return this; }
    virtual IPassivePacketSource *getProvider(cGate *gate) override { return this; }

    virtual bool supportsPacketPushing(cGate *gate) const override { return true; }
    virtual bool supportsPacketPulling(cGate *gate) const override { return true; }

    virtual bool canPushSomePacket(cGate *gate) const override;
    virtual bool canPushPacket(Packet *packet, cGate *gate) const override;

    virtual bool canPullSomePacket(cGate *gate) const override;
    virtual Packet *canPullPacket(cGate *gate) const override;

    virtual void handleCanPushPacketChanged(cGate *gate) override;
    virtual void handleCanPullPacket(cGate *gate) override;
};

} // namespace queueing
} // namespace inet

#endif // ifndef __INET_PACKETGATEBASE_H

