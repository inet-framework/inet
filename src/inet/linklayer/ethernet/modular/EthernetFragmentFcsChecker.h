//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ETHERNETFRAGMENTFCSCHECKER_H
#define __INET_ETHERNETFRAGMENTFCSCHECKER_H

#include "inet/protocolelement/checksum/base/FcsCheckerBase.h"

namespace inet {

using namespace inet::queueing;

class INET_API EthernetFragmentFcsChecker : public FcsCheckerBase
{
  protected:
    uint32_t lastFragmentCompleteFcs = 0;
    mutable uint32_t currentFragmentCompleteFcs = 0;

  protected:
    virtual bool checkComputedFcs(const Packet *packet, uint32_t fcs) const override;
    virtual bool checkFcs(const Packet *packet, FcsMode fcsMode, uint32_t fcs) const override;
    virtual void processPacket(Packet *packet) override;
    virtual void dropPacket(Packet *packet) override;

  public:
    virtual bool matchesPacket(const Packet *packet) const override;
};

} // namespace inet

#endif

