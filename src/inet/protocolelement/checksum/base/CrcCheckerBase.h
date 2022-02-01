//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_CRCCHECKERBASE_H
#define __INET_CRCCHECKERBASE_H

#include "inet/queueing/base/PacketFilterBase.h"
#include "inet/transportlayer/common/CrcMode_m.h"

namespace inet {

using namespace inet::queueing;

class INET_API CrcCheckerBase : public PacketFilterBase
{
  protected:
    virtual bool checkDisabledCrc(const Packet *packet, uint16_t crc) const;
    virtual bool checkDeclaredCorrectCrc(const Packet *packet, uint16_t crc) const;
    virtual bool checkDeclaredIncorrectCrc(const Packet *packet, uint16_t crc) const;
    virtual bool checkComputedCrc(const Packet *packet, uint16_t crc) const;
    virtual bool checkCrc(const Packet *packet, CrcMode crcMode, uint16_t crc) const;
};

} // namespace inet

#endif

