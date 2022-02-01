//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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
            labelsTag->appendLabels(labels[i].c_str());
        }
    }
}

} // namespace queueing
} // namespace inet

