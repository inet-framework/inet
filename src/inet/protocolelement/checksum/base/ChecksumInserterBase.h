//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_CHECKSUMINSERTERBASE_H
#define __INET_CHECKSUMINSERTERBASE_H

#include "inet/queueing/base/PacketFlowBase.h"
#include "inet/common/checksum/ChecksumType_m.h"
#include "inet/common/checksum/ChecksumMode_m.h"

namespace inet {

using namespace inet::queueing;

class INET_API ChecksumInserterBase : public PacketFlowBase
{
  protected:
    ChecksumType checksumType = CHECKSUM_TYPE_UNDEFINED;
    ChecksumMode checksumMode = CHECKSUM_MODE_UNDEFINED;

  protected:
    virtual void initialize(int stage) override;

    virtual uint64_t computeDisabledChecksum(const Packet *packet) const;
    virtual uint64_t computeDeclaredCorrectChecksum(const Packet *packet) const;
    virtual uint64_t computeDeclaredIncorrectChecksum(const Packet *packet) const;
    virtual uint64_t computeComputedChecksum(const Packet *packet, ChecksumType checksumType) const;
    virtual uint64_t computeChecksum(const Packet *packet, ChecksumMode checksumMode, ChecksumType checksumType) const;
};

} // namespace inet

#endif

