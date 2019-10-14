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

#ifndef __INET_PACKETBUFFER_H
#define __INET_PACKETBUFFER_H

#include "inet/queueing/base/PacketQueueingElementBase.h"
#include "inet/queueing/contract/IPacketBuffer.h"
#include "inet/queueing/contract/IPacketCollection.h"
#include "inet/queueing/contract/IPacketDropperFunction.h"

namespace inet {
namespace queueing {

class INET_API PacketBuffer : public PacketQueueingElementBase, public IPacketBuffer, public IPacketCollection
{
  protected:
    const char *displayStringTextFormat = nullptr;
    int frameCapacity = -1;
    b dataCapacity = b(-1);

    b totalLength = b(0);
    std::vector<Packet *> packets;

    IPacketDropperFunction *packetDropperFunction = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual void updateDisplayString();
    virtual bool isOverloaded();

  public:
    virtual int getMaxNumPackets() override { return frameCapacity; }
    virtual int getNumPackets() override { return packets.size(); }

    virtual b getMaxTotalLength() override { return dataCapacity; }
    virtual b getTotalLength() override { return totalLength; }

    virtual Packet *getPacket(int index) override;
    virtual bool isEmpty() override { return packets.size() == 0; }

    virtual void addPacket(Packet *packet) override;
    virtual void removePacket(Packet *packet) override;

    virtual bool supportsPushPacket(cGate *gate) override { return false; }
    virtual bool supportsPopPacket(cGate *gate) override { return false; }
};

} // namespace queueing
} // namespace inet

#endif // ifndef __INET_PACKETBUFFER_H

