//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ETHERNETFRAGMENTPHYHEADERCHECKER_H
#define __INET_ETHERNETFRAGMENTPHYHEADERCHECKER_H

#include "inet/queueing/base/PacketFilterBase.h"

namespace inet {

namespace physicallayer {

using namespace inet::queueing;

class INET_API EthernetFragmentPhyHeaderChecker : public PacketFilterBase
{
  protected:
    int smdNumber = -1;
    int fragmentNumber = 0;

  protected:
    virtual bool matchesPacket(const Packet *packet) const override;
    virtual void processPacket(Packet *packet) override;
    virtual void dropPacket(Packet *packet) override;
};

} // namespace physicallayer

} // namespace inet

#endif

