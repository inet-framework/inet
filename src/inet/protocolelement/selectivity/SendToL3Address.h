//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SENDTOL3ADDRESS_H
#define __INET_SENDTOL3ADDRESS_H

#include "inet/networklayer/common/L3Address.h"
#include "inet/queueing/base/PacketFlowBase.h"

namespace inet {

using namespace inet::queueing;

class INET_API SendToL3Address : public PacketFlowBase
{
  protected:
    L3Address address;

  protected:
    virtual void initialize(int stage) override;
    virtual void processPacket(Packet *packet) override;
};

} // namespace inet

#endif

