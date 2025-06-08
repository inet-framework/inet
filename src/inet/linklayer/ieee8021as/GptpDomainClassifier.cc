//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee8021as/GptpDomainClassifier.h"

#include "Gptp.h"

namespace inet {

Define_Module(GptpDomainClassifier);

void GptpDomainClassifier::initialize(int stage)
{
    PacketClassifierBase::initialize(stage);
    if (stage == INITSTAGE_LINK_LAYER) {
        // Iterate over all gates and find the ones that are connected to Gptp modules
        for (int i = 0; i < gateSize("out"); i++) {
            auto outGate = gate("out", i);
            auto gptp = dynamic_cast<Gptp *>(outGate->getPathEndGate()->getOwnerModule());
            if (gptp != nullptr) {
                auto domainNumber = gptp->getDomainNumber();
                gptpDomainToGateIndex[domainNumber] = i;
            }
        }
    }
}

int GptpDomainClassifier::classifyPacket(Packet *packet)
{
    const auto& header = packet->peekAtFront<GptpBase>();

    if (gptpDomainToGateIndex.find(header->getDomainNumber()) == gptpDomainToGateIndex.end()) {
        EV_ERROR << "Message " << packet->getFullName() << " arrived with foreign domainNumber "
                 << header->getDomainNumber() << ", dropped\n";
        // Frame should be dropped in the gPTP with a not addressed to us error
        // There might be better ways to handle this error, but dropping this packet here is not possible
        return 0;
    }

    return gptpDomainToGateIndex[header->getDomainNumber()];
}

} // namespace inet

