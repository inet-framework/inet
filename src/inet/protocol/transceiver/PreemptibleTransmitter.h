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

#ifndef __INET_PREEMPTIBLETRANSMITTER_H
#define __INET_PREEMPTIBLETRANSMITTER_H

#include "inet/protocol/transceiver/base/PacketTransmitterBase.h"

namespace inet {

class INET_API PreemptibleTransmitter : public PacketTransmitterBase
{
  protected:
    bps datarate = bps(NaN);

    simtime_t txStartTime = -1;
    cMessage *txEndTimer = nullptr;
    Packet *txPacket = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;

    virtual void startTx(Packet *packet);
    virtual void endTx();
    virtual void abortTx();

    virtual simtime_t calculateDuration(const Packet *packet) const override;
    virtual void scheduleTxEndTimer(Signal *signal, simtime_t timePosition);

  public:
    virtual ~PreemptibleTransmitter();

    virtual bool supportsPacketStreaming(cGate *gate) const override { return true; }

    virtual bool canPushSomePacket(cGate *gate) const override { return txPacket == nullptr; };
    virtual bool canPushPacket(Packet *packet, cGate *gate) const override { return txPacket == nullptr; };
    virtual void pushPacket(Packet *packet, cGate *gate) override;
    virtual void pushPacketStart(Packet *packet, cGate *gate) override;
    virtual void pushPacketEnd(Packet *packet, cGate *gate) override;
    virtual void pushPacketProgress(Packet *packet, cGate *gate, b position, b extraProcessableLength = b(0)) override;

    virtual b getPushPacketProcessedLength(Packet *packet, cGate *gate) override;
};

} // namespace inet

#endif // ifndef __INET_PREEMPTIBLETRANSMITTER_H

