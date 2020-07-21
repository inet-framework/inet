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

#ifndef __INET_SENDTOMACADDRESS_H
#define __INET_SENDTOMACADDRESS_H

#include "inet/linklayer/common/MacAddress.h"
#include "inet/queueing/base/PacketFlowBase.h"

namespace inet {

using namespace inet::queueing;

class INET_API SendToMacAddress : public PacketFlowBase
{
  protected:
    MacAddress address;

  protected:
    virtual void initialize(int stage) override;
    virtual void processPacket(Packet *packet) override;

  public:
    virtual bool canPushSomePacket(cGate *gate) const override { return consumer->canPushSomePacket(outputGate->getPathEndGate()); }
    virtual bool canPushPacket(Packet *packet, cGate *gate) const override { return consumer->canPushPacket(packet, outputGate->getPathEndGate()); }
    virtual void pushPacket(Packet *packet, cGate *gate) override;

    virtual IPassivePacketSink *getConsumer(cGate *gate) override { return nullptr; }
    virtual void handleCanPushPacketChanged(cGate *gate) override;

    virtual void handlePushPacketProcessed(Packet *packet, cGate *gate, bool successful) override;
};

} // namespace inet

#endif // ifndef __INET_SENDTOMACADDRESS_H

