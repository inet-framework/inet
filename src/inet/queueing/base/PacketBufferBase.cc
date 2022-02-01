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

const char *PacketBufferBase::resolveDirective(char directive) const
{
    static std::string result;
    switch (directive) {
        case 'p':
            result = std::to_string(getNumPackets());
            break;
        case 'l':
            result = getTotalLength().str();
            break;
        case 'a':
            result = std::to_string(numAddedPackets);
            break;
        case 'r':
            result = std::to_string(numRemovedPackets);
            break;
        case 'd':
            result = std::to_string(numDroppedPackets);
            break;
        default:
            return PacketProcessorBase::resolveDirective(directive);
    }
    return result.c_str();
}

} // namespace queueing
} // namespace inet

