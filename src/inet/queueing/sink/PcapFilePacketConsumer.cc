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

#include "inet/queueing/sink/PcapFilePacketConsumer.h"

#include "inet/common/ModuleAccess.h"

namespace inet {
namespace queueing {

Define_Module(PcapFilePacketConsumer);

void PcapFilePacketConsumer::initialize(int stage)
{
    PassivePacketSinkBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        pcapWriter.setFlush(par("alwaysFlush"));
        pcapWriter.open(par("filename"), par("snaplen"));
        networkType = static_cast<PcapLinkType>(par("networkType").intValue());
        const char *dirString = par("direction");
        if (*dirString == 0)
            direction = DIRECTION_UNDEFINED;
        else if (!strcmp(dirString, "outbound"))
            direction = DIRECTION_OUTBOUND;
        else if (!strcmp(dirString, "inbound"))
            direction = DIRECTION_INBOUND;
        else
            throw cRuntimeError("invalid direction parameter value: %s", dirString);
    }
    else if (stage == INITSTAGE_QUEUEING) {
        checkPacketOperationSupport(inputGate);
        if (producer != nullptr)
            producer->handleCanPushPacketChanged(inputGate->getPathStartGate());
    }
}

void PcapFilePacketConsumer::finish()
{
    pcapWriter.close();
}

void PcapFilePacketConsumer::pushPacket(Packet *packet, cGate *gate)
{
    Enter_Method("pushPacket");
    take(packet);
    emit(packetPushedSignal, packet);
    pcapWriter.writePacket(simTime(), packet, direction, getContainingNicModule(this), networkType);
    numProcessedPackets++;
    processedTotalLength += packet->getDataLength();
    delete packet;
}

} // namespace queueing
} // namespace inet

