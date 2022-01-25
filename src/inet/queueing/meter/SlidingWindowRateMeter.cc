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

#include "inet/queueing/meter/SlidingWindowRateMeter.h"

#include "inet/common/ModuleAccess.h"
#include "inet/queueing/common/RateTag_m.h"

namespace inet {
namespace queueing {

Define_Module(SlidingWindowRateMeter);

void SlidingWindowRateMeter::initialize(int stage)
{
    PacketMeterBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        timeWindow = par("timeWindow");
        WATCH(packetrate);
        WATCH(datarate);
    }
}

void SlidingWindowRateMeter::meterPacket(Packet *packet)
{
    auto now = simTime();
    currentNumPackets++;
    currentTotalPacketLength += packet->getDataLength();
    packetLengths[now] += packet->getDataLength();
    for (auto it = packetLengths.begin(); it != packetLengths.end();) {
        if (it->first < now - timeWindow) {
            currentNumPackets--;
            currentTotalPacketLength -= it->second;
            it = packetLengths.erase(it);
        }
        else
            break;
    }
    datarate = currentTotalPacketLength / s(timeWindow.dbl());
    packetrate = currentNumPackets /timeWindow.dbl();
    auto rateTag = packet->addTagIfAbsent<RateTag>();
    rateTag->setDatarate(datarate);
    rateTag->setPacketrate(packetrate);
}

} // namespace queueing
} // namespace inet

