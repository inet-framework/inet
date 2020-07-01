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

#include "inet/protocol/transceiver/StreamerReceiver.h"

namespace inet {

Define_Module(StreamerReceiver);

void StreamerReceiver::initialize(int stage)
{
    PacketReceiverBase::initialize(stage);
    OperationalMixin::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        datarate = bps(par("datarate"));
}

StreamerReceiver::~StreamerReceiver()
{
    delete rxSignal;
}

void StreamerReceiver::handleMessageWhenUp(cMessage *message)
{
    if (message->getArrivalGate() == inputGate)
        receiveFromMedium(message);
    else
        PacketReceiverBase::handleMessage(message);
}

void StreamerReceiver::handleMessageWhenDown(cMessage *msg)
{
    if (!msg->isSelfMessage()) {
        // received on input gate from another network node
        EV << "Interface is turned off, dropping message (" << msg->getClassName() << ")" << msg->getName() << "\n";
        delete msg;
    }
    else
        OperationalMixin::handleMessageWhenDown(msg);
}

void StreamerReceiver::receivePacketStart(cPacket *cpacket, cGate *gate, double datarate)
{
    ASSERT(rxSignal == nullptr);
    take(cpacket);
    rxSignal = check_and_cast<Signal *>(cpacket);
    emit(receptionStartedSignal, rxSignal);
    auto packet = decodePacket(rxSignal);
    pushOrSendPacketStart(packet, outputGate, consumer, bps(datarate));
}

void StreamerReceiver::receivePacketProgress(cPacket *cpacket, cGate *gate, double datarate, int bitPosition, simtime_t timePosition, int extraProcessableBitLength, simtime_t extraProcessableDuration)
{
    take(cpacket);
    if (rxSignal) {
        delete rxSignal;
        rxSignal = check_and_cast<Signal *>(cpacket);
    }
    else {
        EV_WARN << "Signal start doesn't received, drop it";
        delete cpacket;
    }
}

void StreamerReceiver::receivePacketEnd(cPacket *cpacket, cGate *gate, double datarate)
{
    take(cpacket);
    auto signal = check_and_cast<Signal *>(cpacket);
    emit(receptionEndedSignal, signal);
    if (rxSignal) {
        delete rxSignal;
        rxSignal = nullptr;
        auto packet = decodePacket(signal);
        pushOrSendPacketEnd(packet, outputGate, consumer, bps(datarate));
        delete signal;
    }
    else {
        EV_WARN << "Signal start doesn't received, drop it";
        //TODO drop signal
        delete signal;
    }
}

void StreamerReceiver::handleStartOperation(LifecycleOperation *operation)
{
}

void StreamerReceiver::handleStopOperation(LifecycleOperation *operation)
{
    delete rxSignal;
    rxSignal = nullptr;
}

void StreamerReceiver::handleCrashOperation(LifecycleOperation *operation)
{
    delete rxSignal;
    rxSignal = nullptr;
}

} // namespace inet

