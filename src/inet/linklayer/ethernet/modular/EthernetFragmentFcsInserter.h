//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ETHERNETFRAGMENTFCSINSERTER_H
#define __INET_ETHERNETFRAGMENTFCSINSERTER_H

#include "inet/protocolelement/checksum/base/FcsInserterBase.h"

namespace inet {

using namespace inet::queueing;

class INET_API EthernetFragmentFcsInserter : public FcsInserterBase
{
  protected:
    uint32_t lastFragmentCompleteFcs = 0;
    mutable uint32_t currentFragmentCompleteFcs = 0;

  protected:
    virtual uint32_t computeComputedFcs(const Packet *packet) const override;
    virtual uint32_t computeFcs(const Packet *packet, FcsMode fcsMode) const override;
    virtual void processPacket(Packet *packet) override;
    virtual void handlePacketProcessed(Packet *packet) override;
};

} // namespace inet

#endif

