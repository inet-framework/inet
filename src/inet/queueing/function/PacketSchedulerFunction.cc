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

#include "inet/linklayer/common/UserPriorityTag_m.h"
#include "inet/queueing/function/PacketSchedulerFunction.h"

namespace inet {
namespace queueing {

static int schedulePacketByData(const std::vector<IPassivePacketSource *>& sources)
{
    int selectedIndex = -1;
    int selectedData = -1;
    for (int i = 0; i < (int)sources.size(); i++) {
        auto source = sources[i];
        auto module = check_and_cast<cModule *>(source);
        auto packet = source->canPullPacket(module->gate("out"));
        if (packet != nullptr) {
            const auto& dataChunk = packet->peekDataAt<BytesChunk>(B(0), B(1));
            auto data = dataChunk->getBytes().at(0);
            if (data > selectedData) {
                selectedData = data;
                selectedIndex = i;
            }
        }
    }
    return selectedIndex;
}

Register_Packet_Scheduler_Function(PacketDataScheduler, schedulePacketByData);

static int schedulePacketByUserPriority(const std::vector<IPassivePacketSource *>& sources)
{
    int selectedIndex = -1;
    int selectedUserPriority = -1;
    for (int i = 0; i < (int)sources.size(); i++) {
        auto source = sources[i];
        auto module = check_and_cast<cModule *>(source);
        auto packet = source->canPullPacket(module->gate("out"));
        auto userPriorityReq = packet->getTag<UserPriorityReq>();
        if (userPriorityReq->getUserPriority() > selectedUserPriority)
            selectedIndex = i;
    }
    return selectedIndex;
}

Register_Packet_Scheduler_Function(PacketUserPriorityScheduler, schedulePacketByUserPriority);

} // namespace queueing
} // namespace inet

