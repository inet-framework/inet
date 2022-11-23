//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_CRCINSERTERBASE_H
#define __INET_CRCINSERTERBASE_H

#include "inet/protocolelement/base/ProtocolHeaderInserterBase.h"
#include "inet/transportlayer/common/CrcMode_m.h"

namespace inet {

using namespace inet::queueing;

class INET_API CrcInserterBase : public ProtocolHeaderInserterBase
{
  protected:
    CrcMode crcMode = CRC_MODE_UNDEFINED;

  protected:
    virtual void initialize(int stage) override;

    virtual uint16_t computeDisabledCrc(const Packet *packet) const;
    virtual uint16_t computeDeclaredCorrectCrc(const Packet *packet) const;
    virtual uint16_t computeDeclaredIncorrectCrc(const Packet *packet) const;
    virtual uint16_t computeComputedCrc(const Packet *packet) const;
    virtual uint16_t computeCrc(const Packet *packet, CrcMode crcMode) const;
};

} // namespace inet

#endif

