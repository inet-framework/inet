//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ETHERNETFRAGMENTPHYHEADERINSERTER_H
#define __INET_ETHERNETFRAGMENTPHYHEADERINSERTER_H

#include "inet/protocolelement/base/ProtocolHeaderInserterBase.h"

namespace inet {

namespace physicallayer {

class INET_API EthernetFragmentPhyHeaderInserter : public ProtocolHeaderInserterBase
{
  protected:
    uint8_t smdNumber = 0;
    uint8_t fragmentNumber = 0;

  protected:
    virtual void processPacket(Packet *packet) override;
    virtual void handlePacketProcessed(Packet *packet) override;

  public:
    virtual void pushPacketStart(Packet *packet, cGate *gate, bps datarate) override;
};

} // namespace physicallayer

} // namespace inet

#endif

