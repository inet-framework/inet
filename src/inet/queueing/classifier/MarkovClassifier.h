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

#ifndef __INET_MARKOVCLASSIFIER_H
#define __INET_MARKOVCLASSIFIER_H

#include "inet/queueing/base/PacketClassifierBase.h"
#include "inet/queueing/contract/IActivePacketSink.h"
#include "inet/queueing/contract/IPassivePacketSource.h"

namespace inet {
namespace queueing {

class INET_API MarkovClassifier : public PacketClassifierBase, public virtual IActivePacketSink, public virtual IPassivePacketSource
{
  protected:
    IPassivePacketSource *provider = nullptr;
    std::vector<IActivePacketSink *> collectors;

    std::vector<std::vector<double>> transitionProbabilities;
    std::vector<cDynamicExpression> waitIntervals;

    int state;

    cMessage *transitionTimer = nullptr;
    cMessage *waitTimer = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;

    virtual int classifyPacket(Packet *packet) override;
    virtual void scheduleWaitTimer();

  public:
    virtual ~MarkovClassifier();

    virtual IPassivePacketSource *getProvider(cGate *gate) override { return provider; }

    virtual bool supportsPacketPushing(cGate *gate) const override { return true; }
    virtual bool supportsPacketPulling(cGate *gate) const override { return true; }

    virtual bool canPullSomePacket(cGate *gate) const override;
    virtual Packet *canPullPacket(cGate *gate) const override;

    virtual Packet *pullPacket(cGate *gate) override;

    virtual Packet *pullPacketStart(cGate *gate, bps datarate) override { throw cRuntimeError("Invalid operation");  }
    virtual Packet *pullPacketEnd(cGate *gate, bps datarate) override { throw cRuntimeError("Invalid operation");  }
    virtual Packet *pullPacketProgress(cGate *gate, bps datarate, b position, b extraProcessableLength) override { throw cRuntimeError("Invalid operation");  }
    virtual b getPullPacketProcessedLength(Packet *packet, cGate *gate) override { throw cRuntimeError("Invalid operation");  }

    virtual void handleCanPullPacket(cGate *gate) override;
    virtual void handlePullPacketProcessed(Packet *packet, cGate *gate, bool successful) override;

    virtual const char *resolveDirective(char directive) const override;
};

} // namespace queueing
} // namespace inet

#endif // ifndef __INET_MARKOVCLASSIFIERBASE_H

