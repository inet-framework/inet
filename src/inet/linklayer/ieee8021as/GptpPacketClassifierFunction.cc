//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee8021as/GptpPacket_m.h"
#include "inet/queueing/function/PacketClassifierFunction.h"

namespace inet {

static int classifyPacketByGptpDomainNumber(Packet *packet)
{
    const auto& header = packet->peekAtFront<GptpBase>();
    return header->getDomainNumber();
}

Register_Packet_Classifier_Function(GptpDomainNumberClassifier, classifyPacketByGptpDomainNumber);

} // namespace inet

