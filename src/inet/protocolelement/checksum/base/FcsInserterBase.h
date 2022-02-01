//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_FCSINSERTERBASE_H
#define __INET_FCSINSERTERBASE_H

#include "inet/linklayer/common/FcsMode_m.h"
#include "inet/queueing/base/PacketFlowBase.h"

namespace inet {

using namespace inet::queueing;

class INET_API FcsInserterBase : public PacketFlowBase
{
  protected:
    FcsMode fcsMode = FCS_MODE_UNDEFINED;

  protected:
    virtual void initialize(int stage) override;

    virtual uint32_t computeDisabledFcs(const Packet *packet) const;
    virtual uint32_t computeDeclaredCorrectFcs(const Packet *packet) const;
    virtual uint32_t computeDeclaredIncorrectFcs(const Packet *packet) const;
    virtual uint32_t computeComputedFcs(const Packet *packet) const;
    virtual uint32_t computeFcs(const Packet *packet, FcsMode fcsMode) const;
};

} // namespace inet

#endif

