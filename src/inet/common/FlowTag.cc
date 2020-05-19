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

#include "inet/common/FlowTag.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/Simsignals.h"

namespace inet {

void startPacketFlow(cModule *module, Packet *packet, const char *name)
{
    packet->addRegionTagsWhereAbsent<FlowTag>(b(0), packet->getTotalLength());
    packet->mapAllRegionTagsForUpdate<FlowTag>(b(0), packet->getTotalLength(), [&] (b o, b l, const Ptr<FlowTag>& flowTag) {
        for (int i = 0; i < (int)flowTag->getNamesArraySize(); i++)
            if (!strcmp(name, flowTag->getNames(i)))
                throw cRuntimeError("Flow already exists");
        flowTag->insertNames(name);
    });
    cNamedObject details(name);
    module->emit(packetFlowStartedSignal, packet, &details);
}

void endPacketFlow(cModule *module, Packet *packet, const char *name)
{
    packet->mapAllRegionTagsForUpdate<FlowTag>(b(0), packet->getTotalLength(), [&] (b o, b l, const Ptr<FlowTag>& flowTag) {
        for (int i = 0; i < (int)flowTag->getNamesArraySize(); i++)
            if (!strcmp(name, flowTag->getNames(i)))
                return flowTag->eraseNames(i);
    });
    cNamedObject details(name);
    module->emit(packetFlowEndedSignal, packet, &details);
}

} // namespace inet

