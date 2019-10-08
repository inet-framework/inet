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

#ifndef __INET_PACKETSERVERBASE_H
#define __INET_PACKETSERVERBASE_H

#include "inet/queueing/base/PacketSinkBase.h"
#include "inet/queueing/contract/IPacketCollector.h"
#include "inet/queueing/contract/IPacketProducer.h"

namespace inet {
namespace queueing {

class INET_API PacketServerBase : public PacketSinkBase, public IPacketCollector, public IPacketProducer
{
  protected:
    cGate *inputGate = nullptr;
    IPacketProvider *provider = nullptr;

    cGate *outputGate = nullptr;
    IPacketConsumer *consumer = nullptr;

  protected:
    virtual void initialize(int stage) override;

  public:
    virtual IPacketProvider *getProvider(cGate *gate) override { return provider; }
    virtual IPacketConsumer *getConsumer(cGate *gate) override { return consumer; }

    virtual bool supportsPushPacket(cGate *gate) override { return outputGate == gate; }
    virtual bool supportsPopPacket(cGate *gate) override { return inputGate == gate; }
};

} // namespace queueing
} // namespace inet

#endif // ifndef __INET_PACKETSERVERBASE_H

