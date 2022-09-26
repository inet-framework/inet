//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/base/PacketBufferBase.h"

#include "inet/common/Simsignals.h"
#include "inet/common/StringFormat.h"

namespace inet {
namespace queueing {

void PacketBufferBase::initialize(int stage)
{
    PacketProcessorBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        displayStringTextFormat = par("displayStringTextFormat");
        numAddedPackets = 0;
        numRemovedPackets = 0;
        numDroppedPackets = 0;
        WATCH(numAddedPackets);
        WATCH(numRemovedPackets);
        WATCH(numDroppedPackets);
    }
}

void PacketBufferBase::emit(simsignal_t signal, cObject *object, cObject *details)
{
    if (signal == packetAddedSignal)
        numAddedPackets++;
    else if (signal == packetRemovedSignal)
        numRemovedPackets++;
    else if (signal == packetDroppedSignal)
        numDroppedPackets++;
    cSimpleModule::emit(signal, object, details);
}

std::string PacketBufferBase::resolveDirective(char directive) const
{
    switch (directive) {
        case 'p':
            return std::to_string(getNumPackets());
        case 'l':
            return getTotalLength().str();
        case 'a':
            return std::to_string(numAddedPackets);
        case 'r':
            return std::to_string(numRemovedPackets);
        case 'd':
            return std::to_string(numDroppedPackets);
        default:
            return PacketProcessorBase::resolveDirective(directive);
    }
}

} // namespace queueing
} // namespace inet

