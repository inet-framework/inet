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
#include "inet/queueing/server/PacketServer.h"

namespace inet {
namespace queueing {

Define_Module(PacketServer);

void PacketServer::initialize(int stage)
{
    PacketServerBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        processingTimer = new cMessage("ProcessingTimer");
}

void PacketServer::handleMessage(cMessage *message)
{
    if (message == processingTimer) {
        endProcessingPacket();
        if (canStartProcessingPacket()) {
            startProcessingPacket();
            scheduleProcessingTimer();
        }
        updateDisplayString();
    }
    else
        throw cRuntimeError("Unknown message");
}

void PacketServer::scheduleProcessingTimer()
{
    simtime_t processingTime = par("processingTime");
    auto processingBitrate = bps(par("processingBitrate"));
    processingTime += s(packet->getTotalLength() / processingBitrate).get();
    scheduleAfter(processingTime, processingTimer);
}

bool PacketServer::canStartProcessingPacket()
{
    return provider->canPopSomePacket(inputGate->getPathStartGate()) &&
           consumer->canPushSomePacket(outputGate->getPathEndGate());
}

void PacketServer::startProcessingPacket()
{
    packet = provider->popPacket(inputGate->getPathStartGate());
    take(packet);
    packet->setArrival(getId(), inputGate->getId(), simTime());
    EV_INFO << "Processing packet " << packet->getName() << " started." << endl;
}

void PacketServer::endProcessingPacket()
{
    EV_INFO << "Processing packet " << packet->getName() << " ended.\n";
    processedTotalLength += packet->getDataLength();
    emit(packetServedSignal, packet);
    pushOrSendPacket(packet, outputGate, consumer);
    numProcessedPackets++;
    packet = nullptr;
}

void PacketServer::handleCanPushPacket(cGate *gate)
{
    Enter_Method("handleCanPushPacket");
    if (!processingTimer->isScheduled() && canStartProcessingPacket()) {
        startProcessingPacket();
        scheduleProcessingTimer();
    }
}

void PacketServer::handleCanPopPacket(cGate *gate)
{
    Enter_Method("handleCanPopPacket");
    if (!processingTimer->isScheduled() && canStartProcessingPacket()) {
        startProcessingPacket();
        scheduleProcessingTimer();
    }
}

const char *PacketServer::resolveDirective(char directive) const
{
    static std::string result;
    switch (directive) {
        case 's':
            result = processingTimer->isScheduled() ? "processing" : "";
            break;
        default:
            return PacketServerBase::resolveDirective(directive);
    }
    return result.c_str();
}

} // namespace queueing
} // namespace inet

