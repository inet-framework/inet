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

#include "inet/protocol/transceiver/PacketTransmitter.h"

namespace inet {

Define_Module(PacketTransmitter);

void PacketTransmitter::initialize(int stage)
{
    PacketTransmitterBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        datarate = bps(par("datarate"));
        txEndTimer = new cMessage("endTimer");
    }
}

void PacketTransmitter::handleMessage(cMessage *message)
{
    if (message == txEndTimer)
        producer->handlePushPacketProcessed(txPacket, inputGate->getPathStartGate(), true);
    else
        PacketTransmitterBase::handleMessage(message);
}

void PacketTransmitter::pushPacket(Packet *packet, cGate *gate)
{
    Enter_Method("pushPacket");
    txPacket = packet;
    take(txPacket);
    auto signal = createSignal(txPacket);
    sendDelayed(signal, 0, outputGate, signal->getDuration());
    scheduleTxEndTimer(signal);
}

simtime_t PacketTransmitter::calculateDuration(const Packet *packet) const
{
    return packet->getDataLength().get() / datarate.get();
}

void PacketTransmitter::scheduleTxEndTimer(Signal *signal)
{
    if (txEndTimer->isScheduled())
        cancelEvent(txEndTimer);
    scheduleAt(simTime() + signal->getDuration(), txEndTimer);
}

} // namespace inet

