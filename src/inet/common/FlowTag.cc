//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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
        flowTag->appendNames(name);
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

