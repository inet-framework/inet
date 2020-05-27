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

#ifndef __INET_PACKETTRANSMITTER_H
#define __INET_PACKETTRANSMITTER_H

#include "inet/protocol/transceiver/base/PacketTransmitterBase.h"

namespace inet {

class INET_API PacketTransmitter : public PacketTransmitterBase
{
  protected:
    bps datarate = bps(NaN);

    cMessage *txEndTimer = nullptr;
    Packet *txPacket = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;

    virtual clocktime_t calculateDuration(const Packet *packet) const override;
    virtual void scheduleTxEndTimer(Signal *signal);

    virtual void startTx(Packet *packet);
    virtual void endTx();

  public:
    virtual ~PacketTransmitter();

    virtual bool canPushSomePacket(cGate *gate) const override { return !txEndTimer->isScheduled(); }
    virtual bool canPushPacket(Packet *packet, cGate *gate) const override { return canPushSomePacket(gate); }
    virtual void pushPacket(Packet *packet, cGate *gate) override;

    virtual void pushPacketProgress(Packet *packet, cGate *gate, bps datarate, b position, b extraProcessableLength = b(0)) override { throw cRuntimeError("Invalid operation"); }
    virtual b getPushPacketProcessedLength(Packet *packet, cGate *gate) override { throw cRuntimeError("Invalid operation"); }
};

} // namespace inet

#endif // ifndef __INET_PACKETTRANSMITTER_H

