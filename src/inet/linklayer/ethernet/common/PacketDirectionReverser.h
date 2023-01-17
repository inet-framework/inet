//
// Copyright (C) 2021 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PACKETDIRECTIONREVERSER_H
#define __INET_PACKETDIRECTIONREVERSER_H

#include "inet/common/Protocol.h"
#include "inet/queueing/base/PacketFlowBase.h"

namespace inet {

using namespace inet::queueing;

class INET_API PacketDirectionReverser : public PacketFlowBase
{
  protected:
    bool forwardVlan = true;
    bool forwardPcp = true;
    std::vector<const Protocol *> excludeEncapsulationProtocols;

  protected:
    virtual void initialize(int stage) override;
    virtual void processPacket(Packet *packet) override;
};

} // namespace inet

#endif

