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

#include "inet/common/Simsignals.h"
#include "inet/queueing/server/InstantServer.h"

namespace inet {
namespace queueing {

Define_Module(InstantServer);

bool InstantServer::canProcessPacket()
{
    auto inputGatePathStartGate = inputGate->getPathStartGate();
    auto outputGatePathEndGate = outputGate->getPathEndGate();
    if (provider->canPullSomePacket(inputGatePathStartGate) && consumer->canPushSomePacket(outputGatePathEndGate))
        return true;
    else {
        auto packet = provider->canPullPacket(inputGatePathStartGate);
        return packet != nullptr && consumer->canPushPacket(packet, outputGatePathEndGate);
    }
}

void InstantServer::processPacket()
{
    auto packet = provider->pullPacket(inputGate->getPathStartGate());
    take(packet);
    std::string packetName = packet->getName();
    auto packetLength = packet->getDataLength();
    EV_INFO << "Processing packet " << packetName << " started." << endl;
    emit(packetServedSignal, packet);
    pushOrSendPacket(packet, outputGate, consumer);
    processedTotalLength += packetLength;
    numProcessedPackets++;
    EV_INFO << "Processing packet " << packetName << " ended.\n";
}

void InstantServer::handleCanPushPacketChanged(cGate *gate)
{
    Enter_Method("handleCanPushPacketChanged");
    while (canProcessPacket())
        processPacket();
    updateDisplayString();
}

void InstantServer::handleCanPullPacket(cGate *gate)
{
    Enter_Method("handleCanPullPacket");
    while (canProcessPacket())
        processPacket();
    updateDisplayString();
}

} // namespace queueing
} // namespace inet

