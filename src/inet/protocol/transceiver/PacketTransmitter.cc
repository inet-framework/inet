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

PacketTransmitter::~PacketTransmitter()
{
    cancelAndDelete(txEndTimer);
    delete txPacket;
}

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
        endTx();
    else
        PacketTransmitterBase::handleMessage(message);
}

void PacketTransmitter::pushPacket(Packet *packet, cGate *gate)
{
    Enter_Method("pushPacket");
    take(packet);
    startTx(packet);
}

void PacketTransmitter::startTx(Packet *packet)
{
    ASSERT(txPacket == nullptr);
    txPacket = packet;
    auto signal = encodePacket(txPacket);
    sendDelayed(signal, 0, outputGate, signal->getDuration());
    scheduleTxEndTimer(signal);
}

void PacketTransmitter::endTx()
{
    producer->handlePushPacketProcessed(txPacket, inputGate->getPathStartGate(), true);
    delete txPacket;
    txPacket = nullptr;
    producer->handleCanPushPacket(inputGate->getPathStartGate());
}

simtime_t PacketTransmitter::calculateDuration(const Packet *packet) const
{
    return packet->getDataLength().get() / datarate.get();
}

void PacketTransmitter::scheduleTxEndTimer(Signal *signal)
{
    scheduleAt(simTime() + signal->getDuration(), txEndTimer);
}

} // namespace inet

