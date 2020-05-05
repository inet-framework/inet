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

#include "inet/protocol/transceiver/PreemptibleTransmitter.h"

namespace inet {

Define_Module(PreemptibleTransmitter);

void PreemptibleTransmitter::initialize(int stage)
{
    PacketTransmitterBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        dataratePar = &par("datarate");
        txEndTimer = new cMessage("TxEndTimer");
    }
}

PreemptibleTransmitter::~PreemptibleTransmitter()
{
    cancelAndDelete(txEndTimer);
}

void PreemptibleTransmitter::handleMessage(cMessage *message)
{
    if (message == txEndTimer)
        endTx();
    else
        throw cRuntimeError("Unknown message");
}

void PreemptibleTransmitter::pushPacket(Packet *packet, cGate *gate)
{
    Enter_Method("pushPacket");
    take(packet);
    if (isTransmitting())
        abortTx();
    startTx(packet);
}

void PreemptibleTransmitter::pushPacketStart(Packet *packet, cGate *gate, bps datarate)
{
    Enter_Method("pushPacketStart");
    take(packet);
    startTx(packet);
}

void PreemptibleTransmitter::pushPacketEnd(Packet *packet, cGate *gate, bps datarate)
{
    Enter_Method("pushPacketEnd");
    take(packet);
    delete txPacket;
    txPacket = packet;
    auto signal = encodePacket(txPacket);
    cancelEvent(txEndTimer);
    scheduleTxEndTimer(signal);
    delete signal;
}

void PreemptibleTransmitter::pushPacketProgress(Packet *packet, cGate *gate, bps datarate, b position, b extraProcessableLength)
{
    Enter_Method("pushPacketProgress");
    take(packet);
    delete txPacket;
    txPacket = packet;
    simtime_t timePosition = simTime() - txStartTime;
    int bitPosition = std::floor(datarate.get() * timePosition.dbl());
    auto signal = encodePacket(txPacket);
    sendPacketProgress(signal, outputGate, 0, signal->getDuration(), bps(datarate).get(), bitPosition, timePosition);
    cancelEvent(txEndTimer);
    scheduleTxEndTimer(signal);
}

void PreemptibleTransmitter::startTx(Packet *packet)
{
    ASSERT(txPacket == nullptr);
    datarate = bps(*dataratePar);
    txPacket = packet;
    txStartTime = simTime();
    auto signal = encodePacket(txPacket);
    EV_INFO << "Starting transmission: packetName = " << txPacket->getName() << ", length = " << txPacket->getTotalLength() << ", duration = " << signal->getDuration() << std::endl;
    scheduleTxEndTimer(signal);
    sendPacketStart(signal, outputGate, 0, signal->getDuration(), bps(datarate).get());
}

void PreemptibleTransmitter::endTx()
{
    EV_INFO << "Ending transmission: packetName = " << txPacket->getName() << std::endl;
    auto signal = encodePacket(txPacket);
    sendPacketEnd(signal, outputGate, 0, signal->getDuration(), bps(datarate).get());
    producer->handlePushPacketProcessed(txPacket, inputGate->getPathStartGate(), true);
    delete txPacket;
    txPacket = nullptr;
    txStartTime = -1;
    producer->handleCanPushPacket(inputGate->getPathStartGate());
}

void PreemptibleTransmitter::abortTx()
{
    cancelEvent(txEndTimer);
    b transmittedLength = getPushPacketProcessedLength(txPacket, inputGate);
    txPacket->eraseAtBack(txPacket->getTotalLength() - transmittedLength);
    auto signal = encodePacket(txPacket);
    EV_INFO << "Aborting transmission: packetName = " << txPacket->getName() << ", length = " << txPacket->getTotalLength() << ", duration = " << signal->getDuration() << std::endl;
    sendPacketEnd(signal, outputGate, 0, signal->getDuration(), bps(datarate).get());
    producer->handlePushPacketProcessed(txPacket, inputGate->getPathStartGate(), false);
    delete txPacket;
    txPacket = nullptr;
    txStartTime = -1;
    producer->handleCanPushPacket(inputGate->getPathStartGate());
}

simtime_t PreemptibleTransmitter::calculateDuration(const Packet *packet) const
{
    return packet->getTotalLength().get() / datarate.get();
}

void PreemptibleTransmitter::scheduleTxEndTimer(Signal *signal)
{
    scheduleAt(txStartTime + signal->getDuration(), txEndTimer);
}

b PreemptibleTransmitter::getPushPacketProcessedLength(Packet *packet, cGate *gate)
{
    simtime_t transmissionDuration = simTime() - txStartTime;
    return b(std::floor(datarate.get() * transmissionDuration.dbl()));
}

} // namespace inet

