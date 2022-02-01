//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee8021q/PcpTrafficClassClassifier.h"

#include "inet/linklayer/common/PcpTag_m.h"

namespace inet {

Define_Module(PcpTrafficClassClassifier);

void PcpTrafficClassClassifier::initialize(int stage)
{
    PacketClassifierBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        mode = par("mode");
        mapping = check_and_cast<cValueArray *>(par("mapping").objectValue());
        defaultGateIndex = par("defaultGateIndex");
    }
}

int PcpTrafficClassClassifier::classifyPacket(Packet *packet)
{
    int pcp = -1;
    switch (*mode) {
        case 'r': {
            auto pcpReq = packet->findTag<PcpReq>();
            if (pcpReq != nullptr)
                pcp = pcpReq->getPcp();
            break;
        }
        case 'i': {
            auto pcpInd = packet->findTag<PcpInd>();
            if (pcpInd != nullptr)
                pcp = pcpInd->getPcp();
            break;
        }
        case 'b': {
            auto pcpReq = packet->findTag<PcpReq>();
            if (pcpReq != nullptr)
                pcp = pcpReq->getPcp();
            else {
                auto pcpInd = packet->findTag<PcpInd>();
                if (pcpInd != nullptr)
                    pcp = pcpInd->getPcp();
            }
            break;
        }
    }
    if (pcp != -1) {
        int numTrafficClasses = gateSize("out");
        auto pcpToGateIndex = check_and_cast<cValueArray *>(mapping->get(pcp).objectValue());
        return pcpToGateIndex->get(numTrafficClasses - 1);
    }
    else
        return defaultGateIndex;
}

} // namespace inet

