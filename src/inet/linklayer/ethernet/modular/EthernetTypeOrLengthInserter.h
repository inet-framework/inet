//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ETHERNETTYPEORLENGTHINSERTER_H
#define __INET_ETHERNETTYPEORLENGTHINSERTER_H

#include "inet/protocolelement/base/ProtocolHeaderInserterBase.h"

namespace inet {

class INET_API EthernetTypeOrLengthInserter : public ProtocolHeaderInserterBase
{
  protected:
    virtual void initialize(int stage) override;
    virtual void processPacket(Packet *packet) override;
};

} // namespace inet

#endif

