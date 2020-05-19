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

#include "inet/common/PacketEventTag.h"

namespace inet {

void insertPacketEvent(const cModule *module, Packet *packet, int kind, simtime_t duration, PacketEvent *packetEvent)
{
    auto simulation = module->getSimulation();
    packet->mapAllRegionTagsForUpdate<PacketEventTag>(b(0), packet->getTotalLength(), [&] (b offset, b length, const Ptr<PacketEventTag>& eventTag) {
        auto packetEventCopy = packetEvent->dup();
        packetEventCopy->setKind(kind);
        packetEventCopy->setModulePath(module->getFullPath().c_str());
        packetEventCopy->setEventNumber(simulation->getEventNumber());
        packetEventCopy->setSimulationTime(simulation->getSimTime());
        packetEventCopy->setDuration(duration);
        packetEventCopy->setPacketLength(packet->getTotalLength());
        eventTag->insertPacketEvents(packetEventCopy);
    });
    delete packetEvent;
}

} // namespace inet

