//
// Copyright (C) 2011 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ethernet/common/EthernetFrameClassifier.h"

#include "inet/linklayer/common/EtherType_m.h"
#include "inet/linklayer/ethernet/common/EthernetMacHeader_m.h"

namespace inet {

Define_Module(EthernetFrameClassifier);

int EthernetFrameClassifier::classifyPacket(Packet *packet)
{
    // FIXME need another way to detect pause frame
    auto header = packet->peekAtFront<EthernetMacHeader>(b(-1), Chunk::PF_ALLOW_NULLPTR | Chunk::PF_ALLOW_INCOMPLETE);
    if (header != nullptr && header->getTypeOrLength() == ETHERTYPE_FLOW_CONTROL)
        return 0;
    else
        return 1;
}

} // namespace inet

