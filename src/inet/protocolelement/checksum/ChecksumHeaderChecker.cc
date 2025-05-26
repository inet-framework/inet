//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/checksum/ChecksumHeaderChecker.h"

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/protocolelement/checksum/header/ChecksumHeader_m.h"
#include "inet/protocolelement/common/AccessoryProtocol.h"

namespace inet {

Define_Module(ChecksumHeaderChecker);

void ChecksumHeaderChecker::initialize(int stage)
{
    ChecksumCheckerBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        headerPosition = parseHeaderPosition(par("headerPosition"));
        registerService(AccessoryProtocol::checksum, nullptr, inputGate);
        registerProtocol(AccessoryProtocol::checksum, nullptr, outputGate);
    }
}

void ChecksumHeaderChecker::processPacket(Packet *packet)
{
    B checksumSize = B(getChecksumSizeInBytes(checksumType));
    popHeader<ChecksumHeader>(packet, headerPosition, checksumSize);
}

bool ChecksumHeaderChecker::matchesPacket(const Packet *packet) const
{
    B checksumSize = B(getChecksumSizeInBytes(checksumType));
    const auto& header = peekHeader<ChecksumHeader>(packet, headerPosition, checksumSize);
    auto checksumMode = header->getChecksumMode();
    auto checksum = header->getChecksum();
    return checkChecksum(packet, checksumMode, checksumType, checksum);
}

} // namespace inet

