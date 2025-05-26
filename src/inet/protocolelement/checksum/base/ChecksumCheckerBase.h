//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_CHECKSUMCHECKERBASE_H
#define __INET_CHECKSUMCHECKERBASE_H

#include "inet/queueing/base/PacketFilterBase.h"
#include "inet/common/checksum/ChecksumType_m.h"
#include "inet/common/checksum/ChecksumMode_m.h"

namespace inet {

using namespace inet::queueing;

class INET_API ChecksumCheckerBase : public PacketFilterBase
{
  protected:
    ChecksumType checksumType = CHECKSUM_TYPE_UNDEFINED;

  protected:
    virtual void initialize(int stage) override;

    virtual bool checkDisabledChecksum(const Packet *packet, uint64_t checksum) const;
    virtual bool checkDeclaredCorrectChecksum(const Packet *packet, uint64_t checksum) const;
    virtual bool checkDeclaredIncorrectChecksum(const Packet *packet, uint64_t checksum) const;
    virtual bool checkComputedChecksum(const Packet *packet, ChecksumType checksumType, uint64_t checksum) const;
    virtual bool checkChecksum(const Packet *packet, ChecksumMode checksumMode, ChecksumType checksumType, uint64_t checksum) const;
};

} // namespace inet

#endif

