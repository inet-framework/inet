//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PACKETTAGGERBASE_H
#define __INET_PACKETTAGGERBASE_H

#include "inet/common/Protocol.h"
#include "inet/queueing/base/PacketMarkerBase.h"

namespace inet {
namespace queueing {

class INET_API PacketTaggerBase : public PacketMarkerBase
{
  protected:
    int dscp = -1;
    int ecn = -1;
    int tos = -1;
    int userPriority = -1;
    int interfaceId = -1;
    int hopLimit = -1;
    int vlanId = -1;
    int pcp = -1;
    W transmissionPower = W(NaN);
    std::vector<const Protocol *> encapsulationProtocols;

  protected:
    virtual void initialize(int stage) override;
    virtual void markPacket(Packet *packet) override;
};

} // namespace queueing
} // namespace inet

#endif

