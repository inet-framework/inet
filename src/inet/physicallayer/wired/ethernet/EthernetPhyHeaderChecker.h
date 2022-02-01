//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ETHERNETPHYHEADERCHECKER_H
#define __INET_ETHERNETPHYHEADERCHECKER_H

#include "inet/queueing/base/PacketFilterBase.h"

namespace inet {

using namespace inet::queueing;

namespace physicallayer {

class INET_API EthernetPhyHeaderChecker : public PacketFilterBase
{
  protected:
    virtual void processPacket(Packet *packet) override;
    virtual void dropPacket(Packet *packet) override;

  public:
    virtual bool matchesPacket(const Packet *packet) const override;
};

} // namespace physicallayer

} // namespace inet

#endif

