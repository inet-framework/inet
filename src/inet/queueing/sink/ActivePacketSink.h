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

#ifndef __INET_ACTIVEPACKETSINK_H
#define __INET_ACTIVEPACKETSINK_H

#include "inet/queueing/base/PacketSinkBase.h"
#include "inet/queueing/contract/IActivePacketSink.h"

namespace inet {
namespace queueing {

class INET_API ActivePacketSink : public PacketSinkBase, public virtual IActivePacketSink
{
  public:
    cGate *inputGate = nullptr;
    IPassivePacketSource *provider = nullptr;

    cPar *collectionIntervalParameter = nullptr;
    cMessage *collectionTimer = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;

    virtual void scheduleCollectionTimer();
    virtual void collectPacket();

  public:
    virtual ~ActivePacketSink() { cancelAndDelete(collectionTimer); }

    virtual IPassivePacketSource *getProvider(cGate *gate) override { return provider; }

    virtual bool supportsPacketPushing(cGate *gate) const override { return false; }
    virtual bool supportsPacketPulling(cGate *gate) const override { return inputGate == gate; }

    virtual void handleCanPullPacket(cGate *gate) override;
    virtual void handlePullPacketProcessed(Packet *packet, cGate *gate, bool successful) override;
};

} // namespace queueing
} // namespace inet

#endif // ifndef __INET_ACTIVEPACKETSINK_H

