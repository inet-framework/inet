//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_FRAGMENTNUMBERHEADERCHECKER_H
#define __INET_FRAGMENTNUMBERHEADERCHECKER_H

#include "inet/protocolelement/common/HeaderPosition.h"
#include "inet/queueing/base/PacketFlowBase.h"

namespace inet {

using namespace inet::queueing;

class INET_API FragmentNumberHeaderChecker : public PacketFlowBase
{
  protected:
    HeaderPosition headerPosition = HP_UNDEFINED;

  protected:
    virtual void initialize(int stage) override;

  public:
    virtual void processPacket(Packet *packet) override;
};

} // namespace inet

#endif

