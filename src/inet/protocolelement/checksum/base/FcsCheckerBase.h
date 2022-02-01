//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_FCSCHECKERBASE_H
#define __INET_FCSCHECKERBASE_H

#include "inet/linklayer/common/FcsMode_m.h"
#include "inet/queueing/base/PacketFilterBase.h"

namespace inet {

using namespace inet::queueing;

class INET_API FcsCheckerBase : public PacketFilterBase
{
  protected:
    virtual bool checkDisabledFcs(const Packet *packet, uint32_t fcs) const;
    virtual bool checkDeclaredCorrectFcs(const Packet *packet, uint32_t fcs) const;
    virtual bool checkDeclaredIncorrectFcs(const Packet *packet, uint32_t fcs) const;
    virtual bool checkComputedFcs(const Packet *packet, uint32_t fcs) const;
    virtual bool checkFcs(const Packet *packet, FcsMode fcsMode, uint32_t fcs) const;
};

} // namespace inet

#endif

