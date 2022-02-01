//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PACKETMETERBASE_H
#define __INET_PACKETMETERBASE_H

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/queueing/base/PacketFlowBase.h"
#include "inet/queueing/contract/IPacketMeter.h"

namespace inet {
namespace queueing {

class INET_API PacketMeterBase : public PacketFlowBase, public TransparentProtocolRegistrationListener, public virtual IPacketMeter
{
  protected:
    virtual cGate *getRegistrationForwardingGate(cGate *gate) override;

    virtual void processPacket(Packet *packet) override;
};

} // namespace queueing
} // namespace inet

#endif

