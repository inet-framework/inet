//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_FRAGMENTNUMBERHEADERINSERTER_H
#define __INET_FRAGMENTNUMBERHEADERINSERTER_H

#include "inet/protocolelement/base/ProtocolHeaderInserterBase.h"
#include "inet/protocolelement/common/HeaderPosition.h"

namespace inet {

class INET_API FragmentNumberHeaderInserter : public ProtocolHeaderInserterBase
{
  protected:
    HeaderPosition headerPosition = HP_UNDEFINED;

  protected:
    virtual void initialize(int stage) override;
    virtual void processPacket(Packet *packet) override;
};

} // namespace inet

#endif

