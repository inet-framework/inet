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
    delete txSignal;
    txSignal = nullptr;
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
    ASSERT(txSignal == nullptr);
    txSignal = encodePacket(packet);
    emit(transmissionStartedSignal, txSignal);
    sendDelayed(txSignal->dup(), 0, outputGate, txSignal->getDuration());
    scheduleTxEndTimer(txSignal);
}

void PacketTransmitter::endTx()
{
    emit(transmissionStartedSignal, txSignal);
    auto packet = check_and_cast<Packet *>(txSignal->getEncapsulatedPacket());
    producer->handlePushPacketProcessed(packet, inputGate->getPathStartGate(), true);
    delete txSignal;
    txSignal = nullptr;
    producer->handleCanPushPacket(inputGate->getPathStartGate());
}

clocktime_t PacketTransmitter::calculateDuration(const Packet *packet) const
{
    return packet->getDataLength().get() / datarate.get();
}

void PacketTransmitter::scheduleTxEndTimer(Signal *signal)
{
    scheduleClockEvent(getClockTime() + SIMTIME_AS_CLOCKTIME(signal->getDuration()), txEndTimer);
}

} // namespace inet

