//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/scheduler/ContentBasedScheduler.h"

namespace inet {
namespace queueing {

Define_Module(ContentBasedScheduler);

ContentBasedScheduler::~ContentBasedScheduler()
{
    for (auto filter : filters)
        delete filter;
}

void ContentBasedScheduler::initialize(int stage)
{
    PacketSchedulerBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        defaultGateIndex = par("defaultGateIndex");
        auto packetFilters = check_and_cast<cValueArray *>(par("packetFilters").objectValue());
        for (int i = 0; i < packetFilters->size(); i++) {
            auto filter = new PacketFilter();
            filter->setExpression((cValue&)packetFilters->get(i));
            filters.push_back(filter);
        }
    }
}

int ContentBasedScheduler::schedulePacket()
{
    for (auto filter : filters) {
        for (int i = 0; i < (int)inputGates.size(); i++) {
            auto packet = providers[i]->canPullPacket(inputGates[i]);
            if (packet != nullptr && filter->matches(packet))
                return i;
        }
    }
    return defaultGateIndex;
}

} // namespace queueing
} // namespace inet

