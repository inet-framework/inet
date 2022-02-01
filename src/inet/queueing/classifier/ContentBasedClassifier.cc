//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/classifier/ContentBasedClassifier.h"

namespace inet {
namespace queueing {

Define_Module(ContentBasedClassifier);

ContentBasedClassifier::~ContentBasedClassifier()
{
    for (auto filter : filters)
        delete filter;
}

void ContentBasedClassifier::initialize(int stage)
{
    PacketClassifierBase::initialize(stage);
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

int ContentBasedClassifier::classifyPacket(Packet *packet)
{
    for (int i = 0; i < (int)filters.size(); i++) {
        auto filter = filters[i];
        if (filter->matches(packet))
            return i;
    }
    return defaultGateIndex;
}

} // namespace queueing
} // namespace inet

