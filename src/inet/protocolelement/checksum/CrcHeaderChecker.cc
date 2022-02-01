//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/checksum/CrcHeaderChecker.h"

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/protocolelement/checksum/header/CrcHeader_m.h"
#include "inet/protocolelement/common/AccessoryProtocol.h"

namespace inet {

Define_Module(CrcHeaderChecker);

void CrcHeaderChecker::initialize(int stage)
{
    CrcCheckerBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        headerPosition = parseHeaderPosition(par("headerPosition"));
        registerService(AccessoryProtocol::crc, nullptr, inputGate);
        registerProtocol(AccessoryProtocol::crc, nullptr, outputGate);
    }
}

void CrcHeaderChecker::processPacket(Packet *packet)
{
    popHeader<CrcHeader>(packet, headerPosition, B(2));
}

bool CrcHeaderChecker::matchesPacket(const Packet *packet) const
{
    const auto& header = peekHeader<CrcHeader>(packet, headerPosition, B(2));
    auto crcMode = header->getCrcMode();
    auto crc = header->getCrc();
    return checkCrc(packet, crcMode, crc);
}

} // namespace inet

