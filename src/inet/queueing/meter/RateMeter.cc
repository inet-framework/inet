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

#include "inet/common/ModuleAccess.h"
#include "inet/queueing/common/RateTag_m.h"
#include "inet/queueing/meter/RateMeter.h"

namespace inet {
namespace queueing {

Define_Module(RateMeter);

void RateMeter::initialize(int stage)
{
    PacketMeterBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        alpha = par("alpha");
}

void RateMeter::meterPacket(Packet *packet)
{
    auto now = simTime();
    if (now != lastUpdate) {
        auto elapsedTime = (now - lastUpdate).dbl();
        auto packetrateChange = (currentNumPackets / elapsedTime) - packetrate;
        packetrate = packetrate + packetrateChange * alpha;
        auto datarateChange = (currentTotalPacketLength / s(elapsedTime)) - datarate;
        datarate = datarate + datarateChange * alpha;
        currentNumPackets = 0;
        currentTotalPacketLength = b(0);
        lastUpdate = now;
    }
    currentNumPackets++;
    currentTotalPacketLength += packet->getTotalLength();
    auto rateTag = packet->addTagIfAbsent<RateTag>();
    rateTag->setDatarate(datarate);
    rateTag->setPacketrate(packetrate);
}

} // namespace queueing
} // namespace inet

