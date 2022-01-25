//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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

#include "inet/queueing/server/InstantServer.h"

#include "inet/common/Simsignals.h"

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
    emit(packetPulledSignal, packet);
    std::string packetName = packet->getName();
    auto packetLength = packet->getDataLength();
    EV_INFO << "Processing packet started" << EV_FIELD(packet) << EV_ENDL;
    emit(packetPushedSignal, packet);
    pushOrSendPacket(packet, outputGate, consumer);
    processedTotalLength += packetLength;
    numProcessedPackets++;
    EV_INFO << "Processing packet ended" << EV_ENDL;
}

void InstantServer::handleCanPushPacketChanged(cGate *gate)
{
    Enter_Method("handleCanPushPacketChanged");
    processPackets();
}

void InstantServer::handleCanPullPacketChanged(cGate *gate)
{
    Enter_Method("handleCanPullPacketChanged");
    processPackets();
}

void InstantServer::processPackets()
{
    if (!isProcessing) {
        isProcessing = true;
        while (canProcessPacket())
            processPacket();
        isProcessing = false;
        updateDisplayString();
    }
}

} // namespace queueing
} // namespace inet

