//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/transceiver/DestreamingReceiver.h"

namespace inet {

Define_Module(DestreamingReceiver);

void DestreamingReceiver::initialize(int stage)
{
    StreamingReceiverBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        setTxUpdateSupport(true);
        inputGate->setDeliverImmediately(true);
    }
}

void DestreamingReceiver::handleMessageWhenUp(cMessage *message)
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

void DestreamingReceiver::sendToUpperLayer(Packet *packet)
{
    pushOrSendPacket(packet, outputGate, consumer);
}

void DestreamingReceiver::receivePacketStart(cPacket *cpacket, cGate *gate, bps datarate)
{
    ASSERT(rxSignal == nullptr);
    take(cpacket);
    auto signal = check_and_cast<Signal *>(cpacket);
    EV_INFO << "Receiving signal start from channel" << EV_FIELD(signal) << EV_ENDL;
    rxSignal = signal;
    emit(receptionStartedSignal, rxSignal);
}

void DestreamingReceiver::receivePacketEnd(cPacket *cpacket, cGate *gate, bps datarate)
{
    take(cpacket);
    auto signal = check_and_cast<Signal *>(cpacket);
    EV_INFO << "Receiving signal end from channel" << EV_FIELD(signal) << EV_ENDL;
    delete rxSignal;
    rxSignal = signal;
    emit(receptionEndedSignal, rxSignal);
    auto packet = decodePacket(rxSignal);
    handlePacketProcessed(packet);
    sendToUpperLayer(packet);
    delete rxSignal;
    rxSignal = nullptr;
}

void DestreamingReceiver::receivePacketProgress(cPacket *cpacket, cGate *gate, bps datarate, b position, simtime_t timePosition, b extraProcessableLength, simtime_t extraProcessableDuration)
{
    take(cpacket);
    auto signal = check_and_cast<Signal *>(cpacket);
    EV_INFO << "Receiving signal progress from channel" << EV_FIELD(signal) << EV_ENDL;
    delete rxSignal;
    rxSignal = signal;
}

} // namespace inet

