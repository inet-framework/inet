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

#ifndef __INET_PASSIVEPACKETSOURCE_H
#define __INET_PASSIVEPACKETSOURCE_H

#include "inet/queueing/base/PacketSourceBase.h"
#include "inet/queueing/contract/IActivePacketSink.h"
#include "inet/queueing/contract/IPassivePacketSource.h"

namespace inet {
namespace queueing {

class INET_API PassivePacketSource : public PacketSourceBase, public virtual IPassivePacketSource
{
  protected:
    cGate *outputGate = nullptr;
    IActivePacketSink *collector = nullptr;

    cPar *providingIntervalParameter = nullptr;
    cMessage *providingTimer = nullptr;

    mutable Packet *nextPacket = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;

    virtual void scheduleProvidingTimer();
    virtual Packet *providePacket(cGate *gate);

  public:
    virtual ~PassivePacketSource() { delete nextPacket; cancelAndDelete(providingTimer); }

    virtual bool supportsPacketPushing(cGate *gate) const override { return false; }
    virtual bool supportsPacketPulling(cGate *gate) const override { return outputGate == gate; }

    virtual bool canPullSomePacket(cGate *gate) const override { return !providingTimer->isScheduled(); }
    virtual Packet *canPullPacket(cGate *gate) const override;
    virtual Packet *pullPacket(cGate *gate) override;
};

} // namespace queueing
} // namespace inet

#endif // ifndef __INET_PASSIVEPACKETSOURCE_H

