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

#ifndef __INET_PACKETQUEUEINGELEMENTBASE_H
#define __INET_PACKETQUEUEINGELEMENTBASE_H

#include "inet/queueing/contract/IPassivePacketSink.h"
#include "inet/queueing/contract/IPacketQueueingElement.h"
#include "inet/common/Simsignals.h"

namespace inet {
namespace queueing {

class INET_API PacketQueueingElementBase : public cSimpleModule, public IPacketQueueingElement
{
  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void animateSend(Packet *packet, cGate *gate);
    virtual void checkPacketOperationSupport(cGate *gate) const;

    virtual void pushOrSendPacket(Packet *packet, cGate *gate);
    virtual void pushOrSendPacket(Packet *packet, cGate *gate, IPassivePacketSink *consumer);
    virtual void dropPacket(Packet *packet, PacketDropReason reason, int limit = -1);
};

} // namespace queueing
} // namespace inet

#endif // ifndef __INET_PACKETQUEUEINGELEMENTBASE_H

