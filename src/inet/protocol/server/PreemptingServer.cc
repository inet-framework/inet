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

#include "inet/protocol/server/PreemptingServer.h"

namespace inet {

Define_Module(PreemptingServer);

void PreemptingServer::initialize(int stage)
{
    PacketServerBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        datarate = bps(par("datarate"));
}

void PreemptingServer::startSendingPacket()
{
    streamedPacket = provider->pullPacketStart(inputGate->getPathStartGate(), datarate);
    take(streamedPacket);
    EV_INFO << "Sending packet " << streamedPacket->getName() << " started." << endl;
    pushOrSendPacketStart(streamedPacket->dup(), outputGate, consumer, datarate);
    handlePacketProcessed(streamedPacket);
    updateDisplayString();
}

void PreemptingServer::endSendingPacket()
{
    EV_INFO << "Sending packet " << streamedPacket->getName() << " ended.\n";
    pushOrSendPacketEnd(streamedPacket, outputGate, consumer, datarate);
    streamedPacket = nullptr;
}

void PreemptingServer::handleCanPushPacket(cGate *gate)
{
    Enter_Method("handleCanPushPacket");
    if (streamedPacket != nullptr)
        endSendingPacket();
    if (provider->canPullSomePacket(inputGate->getPathStartGate()))
        startSendingPacket();
}

void PreemptingServer::handleCanPullPacket(cGate *gate)
{
    Enter_Method("handleCanPullPacket");
    if (streamedPacket != nullptr) {
        delete streamedPacket;
        streamedPacket = provider->pullPacketEnd(inputGate->getPathStartGate(), datarate);
        pushOrSendPacketEnd(streamedPacket, outputGate, consumer, datarate);
        streamedPacket = nullptr;
    }
    else if (consumer->canPushSomePacket(outputGate->getPathEndGate()) && provider->canPullSomePacket(inputGate->getPathStartGate()))
        startSendingPacket();
}

void PreemptingServer::handlePushPacketProcessed(Packet *packet, cGate *gate, bool successful)
{
    Enter_Method("handlePushPacketProcessed");
    if (streamedPacket != nullptr) {
        delete streamedPacket;
        streamedPacket = provider->pullPacketEnd(inputGate->getPathStartGate(), datarate);
        delete streamedPacket;
        streamedPacket = nullptr;
    }
}

} // namespace inet
