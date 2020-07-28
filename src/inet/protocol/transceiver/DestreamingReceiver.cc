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

#include "inet/protocol/transceiver/DestreamingReceiver.h"

namespace inet {

Define_Module(DestreamingReceiver);

void DestreamingReceiver::initialize(int stage)
{
    PacketReceiverBase::initialize(stage);
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
            receivePacketStart(signal, inputGate, datarate);
        else if (signal->getRemainingDuration() == 0)
            receivePacketEnd(signal, inputGate, datarate);
        else {
            auto packet = check_and_cast<Packet *>(signal->getEncapsulatedPacket());
            simtime_t timePosition = signal->getDuration() - signal->getRemainingDuration();
            b position = packet->getTotalLength() * timePosition.dbl() / signal->getDuration().dbl();
            receivePacketProgress(signal, inputGate, datarate, position, timePosition, b(0), 0);
        }
    }
    else
        PacketReceiverBase::handleMessage(message);
}

void DestreamingReceiver::sendToUpperLayer(Packet *packet)
{
    packet->setOrigPacketId(-1);
    pushOrSendPacket(packet, outputGate, consumer);
}

void DestreamingReceiver::receivePacketStart(cPacket *cpacket, cGate *gate, bps datarate)
{
    ASSERT(rxSignal == nullptr);
    take(cpacket);
    rxSignal = check_and_cast<Signal *>(cpacket);
    emit(receptionStartedSignal, rxSignal);
}

void DestreamingReceiver::receivePacketEnd(cPacket *cpacket, cGate *gate, bps datarate)
{
    delete rxSignal;
    rxSignal = check_and_cast<Signal *>(cpacket);
    emit(receptionEndedSignal, rxSignal);
    auto packet = decodePacket(rxSignal);
    sendToUpperLayer(packet);
    delete rxSignal;
    rxSignal = nullptr;
}

void DestreamingReceiver::receivePacketProgress(cPacket *cpacket, cGate *gate, bps datarate, b position, simtime_t timePosition, b extraProcessableLength, simtime_t extraProcessableDuration)
{
    take(cpacket);
    delete rxSignal;
    rxSignal = check_and_cast<Signal *>(cpacket);
}

} // namespace inet

