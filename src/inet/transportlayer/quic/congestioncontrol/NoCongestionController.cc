//
// Copyright (C) 2019-2024 Timo Völker, Ekaterina Volodina
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "NoCongestionController.h"

namespace inet {
namespace quic {

NoCongestionController::NoCongestionController()
{
}

NoCongestionController::~NoCongestionController()
{
}

void NoCongestionController::onPacketAcked(QuicPacket *ackedPacket)
{

}

void NoCongestionController::onPacketSent(QuicPacket *sentPacket)
{

}

void NoCongestionController::onPacketsLost(std::vector<QuicPacket *> *lostPackets, bool inPersistentCongestion)
{

}

uint32_t NoCongestionController::getRemainingCongestionWindow()
{
    return 1 << 16; // 2^16 is the max IP size according to the length field
}

void NoCongestionController::setAppLimited(bool appLimited)
{

}

} /* namespace quic */
} /* namespace inet */
