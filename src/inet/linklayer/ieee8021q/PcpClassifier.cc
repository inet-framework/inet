//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee8021q/PcpClassifier.h"

#include "inet/linklayer/common/PcpTag_m.h"

namespace inet {

Define_Module(PcpClassifier);

void PcpClassifier::initialize(int stage)
{
    PacketClassifierBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        mode = par("mode");
        pcpToGateIndex = check_and_cast<cValueArray *>(par("pcpToGateIndex").objectValue());
        defaultGateIndex = par("defaultGateIndex");
    }
}

int PcpClassifier::classifyPacket(Packet *packet)
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
    if (pcp != -1)
        return pcpToGateIndex->get(pcp).intValue();
    else
        return defaultGateIndex;
}

} // namespace inet

