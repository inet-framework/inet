//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ETHERNETPHYHEADERINSERTER_H
#define __INET_ETHERNETPHYHEADERINSERTER_H

#include "inet/queueing/base/PacketFlowBase.h"

namespace inet {

namespace physicallayer {

using namespace inet::queueing;

class INET_API EthernetPhyHeaderInserter : public PacketFlowBase
{
  protected:
    virtual void processPacket(Packet *packet) override;

  public:
    virtual void pushPacketStart(Packet *packet, cGate *gate, bps datarate) override;
};

} // namespace physicallayer

} // namespace inet

#endif

