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

#ifndef __INET_COMPOUNDPACKETQUEUE_H
#define __INET_COMPOUNDPACKETQUEUE_H

#include "inet/queueing/base/PacketQueueBase.h"
#include "inet/queueing/contract/IPacketCollection.h"
#include "inet/queueing/contract/IPassivePacketSink.h"
#include "inet/queueing/contract/IPassivePacketSource.h"

namespace inet {
namespace queueing {

class INET_API CompoundPacketQueue : public PacketQueueBase, public cListener
{
  protected:
    int packetCapacity = -1;
    b dataCapacity = b(-1);

    cGate *inputGate = nullptr;
    IPassivePacketSink *consumer = nullptr;

    cGate *outputGate = nullptr;
    IPassivePacketSource *provider = nullptr;
    IPacketCollection *collection = nullptr;

  protected:
    virtual void initialize(int stage) override;

  public:
    virtual int getMaxNumPackets() const override { return packetCapacity; }
    virtual int getNumPackets() const override { return collection->getNumPackets(); }

    virtual b getMaxTotalLength() const override { return dataCapacity; }
    virtual b getTotalLength() const override { return collection->getTotalLength(); }

    virtual bool isEmpty() const override { return collection->isEmpty(); }
    virtual Packet *getPacket(int index) const override { return collection->getPacket(index); }
    virtual void removePacket(Packet *packet) override;

    virtual bool supportsPushPacket(cGate *gate) const override { return inputGate == gate; }
    virtual bool canPushPacket(Packet *packet, cGate *gate) const override { return true; }
    virtual void pushPacket(Packet *packet, cGate *gate) override;

    virtual bool supportsPopPacket(cGate *gate) const override { return outputGate == gate; }
    virtual Packet *canPopPacket(cGate *gate) const override { throw cRuntimeError("Invalid operation"); }
    virtual Packet *popPacket(cGate *gate) override;

    virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details) override;
};

} // namespace queueing
} // namespace inet

#endif // ifndef __INET_COMPOUNDPACKETQUEUE_H

