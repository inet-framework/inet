//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/checksum/FcsHeaderChecker.h"

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/protocolelement/checksum/header/FcsHeader_m.h"
#include "inet/protocolelement/common/AccessoryProtocol.h"

namespace inet {

Define_Module(FcsHeaderChecker);

void FcsHeaderChecker::initialize(int stage)
{
    FcsCheckerBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        headerPosition = parseHeaderPosition(par("headerPosition"));
        registerService(AccessoryProtocol::fcs, nullptr, inputGate);
        registerProtocol(AccessoryProtocol::fcs, nullptr, outputGate);
    }
}

void FcsHeaderChecker::processPacket(Packet *packet)
{
    popHeader<FcsHeader>(packet, headerPosition, B(4));
}

bool FcsHeaderChecker::matchesPacket(const Packet *packet) const
{
    const auto& header = peekHeader<FcsHeader>(packet, headerPosition, B(4));
    auto fcsMode = header->getFcsMode();
    auto fcs = header->getFcs();
    return checkFcs(packet, fcsMode, fcs);
}

} // namespace inet

