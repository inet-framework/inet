//
// Copyright (C) 2019-2024 Timo Völker
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "DplpmtudProbePacket.h"

namespace inet {
namespace quic {

DplpmtudProbePacket::DplpmtudProbePacket(std::string name, Dplpmtud *dplpmtud) : QuicPacket(name) {
    this->dplpmtud = dplpmtud;
}

DplpmtudProbePacket::~DplpmtudProbePacket() { }

void DplpmtudProbePacket::onPacketLost()
{
    dplpmtud->onProbePacketLost(this->getSize());
}

void DplpmtudProbePacket::onPacketAcked()
{
    dplpmtud->onProbePacketAcked(this->getSize());
}

bool DplpmtudProbePacket::isDplpmtudProbePacket()
{
    return true;
}

} /* namespace quic */
} /* namespace inet */
