//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE8021QSOCKETPACKETPROCESSOR_H
#define __INET_IEEE8021QSOCKETPACKETPROCESSOR_H

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ModuleRefByPar.h"
#include "inet/linklayer/ieee8021q/Ieee8021qSocketTable.h"
#include "inet/queueing/base/PacketPusherBase.h"

namespace inet {

class INET_API Ieee8021qSocketPacketProcessor : public queueing::PacketPusherBase, public TransparentProtocolRegistrationListener
{
  protected:
    ModuleRefByPar<Ieee8021qSocketTable> socketTable;

  protected:
    virtual void initialize(int stage) override;

    virtual cGate *getRegistrationForwardingGate(cGate *gate) override;

  public:
    virtual void pushPacket(Packet *packet, cGate *gate) override;
};

} // namespace inet

#endif

