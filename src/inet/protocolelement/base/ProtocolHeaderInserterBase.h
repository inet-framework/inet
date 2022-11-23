//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PROTOCOLHEADERINSERTERBASE_H
#define __INET_PROTOCOLHEADERINSERTERBASE_H

#include "inet/queueing/base/PacketFlowBase.h"

namespace inet {

class INET_API ProtocolHeaderInserterBase : public queueing::PacketFlowBase
{
  public:
    virtual bool canPushPacket(Packet *packet, cGate *gate) const override;
};

} // namespace inet

#endif

