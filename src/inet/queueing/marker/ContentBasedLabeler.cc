//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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
        auto packetFilters = check_and_cast<cValueArray *>(par("packetFilters").objectValue());
        for (int i = 0; i < packetFilters->size(); i++) {
            auto filter = new PacketFilter();
            filter->setExpression((cValue&)packetFilters->get(i));
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
            labelsTag->appendLabels(labels[i].c_str());
        }
    }
}

} // namespace queueing
} // namespace inet

