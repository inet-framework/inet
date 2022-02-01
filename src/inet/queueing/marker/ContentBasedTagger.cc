//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/marker/ContentBasedTagger.h"

namespace inet {
namespace queueing {

Define_Module(ContentBasedTagger);

void ContentBasedTagger::initialize(int stage)
{
    PacketTaggerBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        filter.setExpression(par("packetFilter").objectValue());
}

void ContentBasedTagger::markPacket(Packet *packet)
{
    if (filter.matches(packet))
        PacketTaggerBase::markPacket(packet);
}

} // namespace queueing
} // namespace inet

