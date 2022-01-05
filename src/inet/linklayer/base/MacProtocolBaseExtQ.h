//
// Copyright (C) 2013 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#ifndef __INET_MACPROTOCOLBASEEXTQ_H
#define __INET_MACPROTOCOLBASEEXTQ_H

#include "inet/linklayer/base/MacProtocolBase.h"

namespace inet {

class INET_API MacProtocolBaseExtQ : public MacProtocolBase
{
  protected:
    /** Currently transmitted frame if any */
    Packet *currentTxFrame = nullptr;

    /** Messages received from upper layer and to be transmitted later */
    opp_component_ptr<queueing::IPacketQueue> txQueue;

  protected:
    MacProtocolBaseExtQ();
    virtual ~MacProtocolBaseExtQ();

    virtual void initialize(int stage) override;

    virtual void deleteCurrentTxFrame();
    virtual void dropCurrentTxFrame(PacketDropDetails& details);

    /**
     * should clear queue and emit signal "packetDropped" with entire packets
     */
    virtual void flushQueue(PacketDropDetails& details);

    /**
     * should clear queue silently
     */
    virtual void clearQueue();

    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

    queueing::IPacketQueue *getQueue(cGate *gate) const;

    virtual bool canDequeuePacket() const;
    virtual Packet *dequeuePacket();
};

} // namespace inet

#endif

