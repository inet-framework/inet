//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/transceiver/PacketReceiver.h"

namespace inet {

Define_Module(PacketReceiver);

void PacketReceiver::handleMessageWhenUp(cMessage *message)
{
    if (message->isPacket())
        receiveSignal(check_and_cast<Signal *>(message));
    else
        PacketReceiverBase::handleMessageWhenUp(message);
    updateDisplayString();
}

void PacketReceiver::receiveSignal(Signal *signal)
{
    EV_INFO << "Receiving signal from channel" << EV_FIELD(signal) << EV_ENDL;
    emit(receptionEndedSignal, signal);
    auto packet = decodePacket(signal);
    handlePacketProcessed(packet);
    pushOrSendPacket(packet, outputGate, consumer);
    delete signal;
}

} // namespace inet

