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

#include "inet/protocolelement/aggregation/base/DeaggregatorBase.h"

namespace inet {

void DeaggregatorBase::initialize(int stage)
{
    PacketPusherBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        deleteSelf = par("deleteSelf");
}

void DeaggregatorBase::pushPacket(Packet *aggregatedPacket, cGate *gate)
{
    Enter_Method("pushPacket");
    take(aggregatedPacket);
    auto subpackets = deaggregatePacket(aggregatedPacket);
    for (auto subpacket : subpackets) {
        EV_INFO << "Deaggregating packet" << EV_FIELD(subpacket) << EV_FIELD(packet, *aggregatedPacket) << EV_ENDL;
        pushOrSendPacket(subpacket, outputGate, consumer);
    }
    processedTotalLength += aggregatedPacket->getDataLength();
    numProcessedPackets++;
    updateDisplayString();
    delete aggregatedPacket;
    if (deleteSelf)
        deleteModule();
}

} // namespace inet

