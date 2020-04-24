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

#include "inet/protocol/transceiver/StreamingReceiver.h"

namespace inet {

Define_Module(StreamingReceiver);

void StreamingReceiver::initialize(int stage)
{
    PacketReceiverBase::initialize(stage);
    OperationalBase0::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        datarate = bps(par("datarate"));
}

StreamingReceiver::~StreamingReceiver()
{
    delete rxSignal;
}

void StreamingReceiver::handleMessageWhenUp(cMessage *message)
{
    if (message->getArrivalGate() == inputGate)
        receiveFromMedium(message);
    else
        PacketReceiverBase::handleMessage(message);
}

void StreamingReceiver::handleMessageWhenDown(cMessage *msg)
{
    if (!msg->isSelfMessage()) {
        // received on input gate from another network node
        EV << "Interface is turned off, dropping message (" << msg->getClassName() << ")" << msg->getName() << "\n";
        delete msg;
    }
    else
        OperationalBase0::handleMessageWhenDown(msg);
}

void StreamingReceiver::sendToUpperLayer(Packet *packet)
{
    pushOrSendPacket(packet, outputGate, consumer);
}

void StreamingReceiver::receivePacketStart(cPacket *cpacket, cGate *gate, double datarate)
{
    ASSERT(rxSignal == nullptr);
    take(cpacket);
    rxSignal = check_and_cast<Signal *>(cpacket);
}

void StreamingReceiver::receivePacketProgress(cPacket *cpacket, cGate *gate, double datarate, int bitPosition, simtime_t timePosition, int extraProcessableBitLength, simtime_t extraProcessableDuration)
{
    take(cpacket);
    delete rxSignal;
    rxSignal = check_and_cast<Signal *>(cpacket);
}

void StreamingReceiver::receivePacketEnd(cPacket *cpacket, cGate *gate, double datarate)
{
    delete rxSignal;
    rxSignal = check_and_cast<Signal *>(cpacket);
    auto packet = decodePacket(rxSignal);
    sendToUpperLayer(packet);
    delete rxSignal;
    rxSignal = nullptr;
}

void StreamingReceiver::handleStartOperation(LifecycleOperation *operation)
{
}

void StreamingReceiver::handleStopOperation(LifecycleOperation *operation)
{
    delete rxSignal;
    rxSignal = nullptr;
}

void StreamingReceiver::handleCrashOperation(LifecycleOperation *operation)
{
    delete rxSignal;
    rxSignal = nullptr;
}

} // namespace inet

