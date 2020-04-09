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
    b minPacketLength = b(-1);
    b roundingLength = b(-1);
    cGate *preemptedOutputGate = nullptr;
    IPassivePacketSink *preemptedConsumer = nullptr;

    Packet *packet = nullptr;

  protected:
    virtual ~PreemptingServer() { delete packet; }
    virtual void initialize(int stage) override;

    virtual void startSendingPacket();
    virtual void endSendingPacket();

    virtual int getPriority(Packet *packet) const;

  public:
    virtual bool supportsPacketPushing(cGate *gate) const override { return preemptedOutputGate == gate || PacketServerBase::supportsPacketPushing(gate); }

    virtual void handleCanPushPacket(cGate *gate) override;
    virtual void handleCanPullPacket(cGate *gate) override;
};

} // namespace inet

#endif // ifndef __INET_PREEMPTINGSERVER_H

