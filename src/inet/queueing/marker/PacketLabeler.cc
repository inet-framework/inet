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

#include "inet/queueing/marker/PacketLabeler.h"

#include "inet/queueing/common/LabelsTag_m.h"

namespace inet {
namespace queueing {

Define_Module(PacketLabeler);

PacketLabeler::~PacketLabeler()
{
    for (auto filter : filters)
        delete filter;
}

void PacketLabeler::initialize(int stage)
{
    PacketLabelerBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        cStringTokenizer tokenizer(par("filterClasses"));
        while (tokenizer.hasMoreTokens()) {
            auto filter = createFilterFunction(tokenizer.nextToken());
            filters.push_back(filter);
        }
    }
}

IPacketFilterFunction *PacketLabeler::createFilterFunction(const char *filterClass) const
{
    return check_and_cast<IPacketFilterFunction *>(createOne(filterClass));
}

void PacketLabeler::markPacket(Packet *packet)
{
    auto labelsTag = packet->addTagIfAbsent<LabelsTag>();
    for (int i = 0; i < (int)filters.size(); i++) {
        auto filter = filters[i];
        if (filter->matchesPacket(packet)) {
            EV_INFO << "Marking packet" << EV_FIELD(label, labels[i]) << EV_FIELD(packet) << EV_ENDL;
            labelsTag->insertLabels(labels[i].c_str());
        }
    }
}

} // namespace queueing
} // namespace inet

