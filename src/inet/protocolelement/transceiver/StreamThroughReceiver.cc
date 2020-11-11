//
// Copyright (C) 2020 OpenSim Ltd.
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

#include "inet/protocolelement/transceiver/StreamThroughReceiver.h"

namespace inet {

Define_Module(StreamThroughReceiver);

void StreamThroughReceiver::initialize(int stage)
{
    StreamingReceiverBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        setTxUpdateSupport(true);
        inputGate->setDeliverImmediately(true);
    }
}

void StreamThroughReceiver::handleMessageWhenUp(cMessage *message)
{
    if (message->getArrivalGate() == inputGate) {
        auto signal = check_and_cast<Signal *>(message);
        if (!signal->isUpdate())
            receivePacketStart(signal, inputGate, rxDatarate);
        else if (signal->getRemainingDuration() == 0)
            receivePacketEnd(signal, inputGate, rxDatarate);
        else {
            auto packet = check_and_cast<Packet *>(signal->getEncapsulatedPacket());
            simtime_t timePosition = signal->getDuration() - signal->getRemainingDuration();
            b position(std::floor(packet->getTotalLength().get() * timePosition.dbl() / signal->getDuration().dbl()));
            receivePacketProgress(signal, inputGate, rxDatarate, position, timePosition, b(0), 0);
        }
    }
    else
        StreamingReceiverBase::handleMessage(message);
    updateDisplayString();
}

void StreamThroughReceiver::receivePacketStart(cPacket *cpacket, cGate *gate, bps datarate)
{
    ASSERT(rxSignal == nullptr);
    take(cpacket);
    rxSignal = check_and_cast<Signal *>(cpacket);
    emit(receptionStartedSignal, rxSignal);
    auto packet = decodePacket(rxSignal);
    pushOrSendPacketStart(packet, outputGate, consumer, datarate, rxSignal->getTransmissionId());
}

void StreamThroughReceiver::receivePacketProgress(cPacket *cpacket, cGate *gate, bps datarate, b position, simtime_t timePosition, b extraProcessableLength, simtime_t extraProcessableDuration)
{
    take(cpacket);
    auto signal = check_and_cast<Signal *>(cpacket);
    if (rxSignal) {
        delete rxSignal;
        rxSignal = signal;
    }
    else {
        EV_WARN << "Dropping signal progress because signal start was not received" << EV_FIELD(signal) << EV_ENDL;
        delete signal;
    }
}

void StreamThroughReceiver::receivePacketEnd(cPacket *cpacket, cGate *gate, bps datarate)
{
    take(cpacket);
    auto signal = check_and_cast<Signal *>(cpacket);
    emit(receptionEndedSignal, signal);
    if (rxSignal) {
        delete rxSignal;
        rxSignal = signal;
        auto packet = decodePacket(rxSignal);
        handlePacketProcessed(packet);
        pushOrSendPacketEnd(packet, outputGate, consumer, rxSignal->getTransmissionId());
        delete rxSignal;
        rxSignal = nullptr;
    }
    else {
        EV_WARN << "Dropping signal end because signal start was not received" << EV_FIELD(signal) << EV_ENDL;
        delete signal;
    }
}

} // namespace inet

