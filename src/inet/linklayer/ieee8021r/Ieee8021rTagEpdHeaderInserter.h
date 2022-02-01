//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE8021RTAGEPDHEADERINSERTER_H
#define __INET_IEEE8021RTAGEPDHEADERINSERTER_H

#include "inet/common/Protocol.h"
#include "inet/queueing/base/PacketFlowBase.h"

namespace inet {

using namespace inet::queueing;

class INET_API Ieee8021rTagEpdHeaderInserter : public PacketFlowBase
{
  protected:
    const Protocol *nextProtocol = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual void processPacket(Packet *packet) override;
};

} // namespace inet

#endif

