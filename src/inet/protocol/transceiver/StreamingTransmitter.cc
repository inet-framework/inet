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

#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/linklayer/ethernet/EtherPhyFrame_m.h"
#include "inet/protocol/transceiver/StreamingTransmitter.h"

namespace inet {

Define_Module(StreamingTransmitter);

void StreamingTransmitter::initialize(int stage)
{
    PacketTransmitterBase::initialize(stage);
    OperationalMixin::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        dataratePar = &par("datarate");
        datarate = bps(*dataratePar);
        txEndTimer = new cMessage("TxEndTimer");
    }
}

StreamingTransmitter::~StreamingTransmitter()
{
    cancelAndDelete(txEndTimer);
    delete txSignal;
    delete txPacket;
}

void StreamingTransmitter::handleMessageWhenUp(cMessage *message)
{
    if (message == txEndTimer) {
        endTx();
        producer->handleCanPushPacket(inputGate->getPathStartGate());
    }
    else
        PacketTransmitterBase::handleMessage(message);
}

void StreamingTransmitter::pushPacket(Packet *packet, cGate *gate)
{
    Enter_Method("pushPacket");
    take(packet);
    ASSERT(txSignal == nullptr);

    datarate = bps(*dataratePar);    // refresh stored datarate before start packet sending
    startTx(packet);
}

void StreamingTransmitter::startTx(Packet *packet)
{
    datarate = bps(*dataratePar);    //TODO for refresh datarate before start packet sending

    ASSERT(txPacket == nullptr);
    ASSERT(txSignal == nullptr);
    txPacket = packet;
    txStartTime = simTime();
    txSignal = encodePacket(txPacket);
    EV_INFO << "Starting transmission: packetName = " << txPacket->getName() << ", length = " << txPacket->getTotalLength() << ", duration = " << txSignal->getDuration() << std::endl;
    scheduleTxEndTimer(txSignal);
    sendPacketStart(txSignal->dup(), outputGate, 0, txSignal->getDuration(), bps(datarate).get());
}

void StreamingTransmitter::endTx()
{
    EV_INFO << "Ending transmission: packetName = " << txPacket->getName() << std::endl;
    sendPacketEnd(txSignal, outputGate, 0, txSignal->getDuration(), bps(datarate).get());
    producer->handlePushPacketProcessed(txPacket, inputGate->getPathStartGate(), true);
    delete txPacket;
    txPacket = nullptr;
    txSignal = nullptr;
    txStartTime = -1;
}

void StreamingTransmitter::abortTx()
{
    ASSERT(txPacket != nullptr);
    ASSERT(txSignal != nullptr);
    cancelEvent(txEndTimer);
    b transmittedLength = getPushPacketProcessedLength(txPacket, inputGate);
    txPacket->eraseAtBack(txPacket->getTotalLength() - transmittedLength);
    auto signal = encodePacket(txPacket);
    EV_INFO << "Aborting transmission: packetName = " << txPacket->getName() << ", length = " << txPacket->getTotalLength() << ", duration = " << signal->getDuration() << std::endl;
    sendPacketEnd(txSignal, outputGate, 0, signal->getDuration(), bps(datarate).get());
    producer->handlePushPacketProcessed(txPacket, inputGate->getPathStartGate(), true);
    txPacket = nullptr;
    delete txSignal;
    txSignal = nullptr;
    txStartTime = -1;
}

simclocktime_t StreamingTransmitter::calculateDuration(const Packet *packet) const
{
    return packet->getDataLength().get() / datarate.get();
}

void StreamingTransmitter::scheduleTxEndTimer(Signal *signal)
{
    if (txEndTimer->isScheduled())
        cancelEvent(txEndTimer);
    scheduleClockEvent(getClockTime() + SIMTIME_AS_CLOCKTIME(signal->getDuration()), txEndTimer);
}

void StreamingTransmitter::pushPacketProgress(Packet *packet, cGate *gate, bps datarate, b position, b extraProcessableLength)
{
    take(packet);
    delete packet;
    // TODO:
}

b StreamingTransmitter::getPushPacketProcessedLength(Packet *packet, cGate *gate)
{
    if (txPacket == nullptr)
        return b(0);
    simtime_t transmissionDuration = simTime() - txStartTime;
    return b(std::floor(datarate.get() * transmissionDuration.dbl()));
}

void StreamingTransmitter::handleStartOperation(LifecycleOperation *operation)
{
    producer->handleCanPushPacket(inputGate->getPathStartGate());
}

void StreamingTransmitter::handleStopOperation(LifecycleOperation *operation)
{
    if (txSignal)
        endTx();
}

void StreamingTransmitter::handleCrashOperation(LifecycleOperation *operation)
{
    if (txSignal)
        abortTx();
}

} // namespace inet

