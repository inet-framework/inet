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

#include "inet/queueing/marker/ContentBasedLabeler.h"

#include "inet/queueing/common/LabelsTag_m.h"

namespace inet {
namespace queueing {

Define_Module(ContentBasedLabeler);

ContentBasedLabeler::~ContentBasedLabeler()
{
    for (auto filter : filters)
        delete filter;
}

void ContentBasedLabeler::initialize(int stage)
{
    PacketLabelerBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        cStringTokenizer packetFiltersTokenizer(par("packetFilters"), ";");
        cStringTokenizer packetDataFiltersTokenizer(par("packetDataFilters"), ";");
        while (packetFiltersTokenizer.hasMoreTokens()) {
            auto packetFilter = packetFiltersTokenizer.nextToken();
            auto packetDataFilter = packetDataFiltersTokenizer.nextToken();
            auto filter = new PacketFilter();
            filter->setPattern(packetFilter, packetDataFilter);
            filters.push_back(filter);
        }
    }
}

void ContentBasedLabeler::markPacket(Packet *packet)
{
    auto labelsTag = packet->addTagIfAbsent<LabelsTag>();
    for (int i = 0; i < (int)filters.size(); i++) {
        auto filter = filters[i];
        if (filter->matches(packet)) {
            EV_INFO << "Marking packet" << EV_FIELD(label, labels[i]) << EV_FIELD(packet) << EV_ENDL;
            labelsTag->insertLabels(labels[i].c_str());
        }
    }
}

} // namespace queueing
} // namespace inet

