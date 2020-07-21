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
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#ifndef __INET_PREEMPTINGSERVER_H
#define __INET_PREEMPTINGSERVER_H

#include "inet/queueing/base/PacketServerBase.h"

namespace inet {

using namespace inet::queueing;

class INET_API PreemptingServer : public PacketServerBase
{
  protected:
    bps datarate = bps(NaN);

    Packet *streamedPacket = nullptr;

    cMessage *timer = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;

    virtual bool isStreaming() const { return streamedPacket != nullptr; }
    virtual bool canStartStreaming() const;

    virtual void startStreaming();
    virtual void endStreaming();

  public:
    virtual ~PreemptingServer() { delete streamedPacket; cancelAndDelete(timer); }

    virtual void handleCanPushPacketChanged(cGate *gate) override;
    virtual void handleCanPullPacketChanged(cGate *gate) override;

    virtual void handlePushPacketProcessed(Packet *packet, cGate *gate, bool successful) override;
};

} // namespace inet

#endif // ifndef __INET_PREEMPTINGSERVER_H

