//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_VLANREQMAPPER_H
#define __INET_VLANREQMAPPER_H

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ModuleRefByPar.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/queueing/base/PacketFlowBase.h"

namespace inet {

using namespace inet::queueing;

class INET_API VlanReqMapper : public PacketFlowBase, public TransparentProtocolRegistrationListener
{
  protected:
    const Protocol *protocol = nullptr;
    ModuleRefByPar<IInterfaceTable> interfaceTable;
    cValueMap *mappedVlanIds = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual void processPacket(Packet *packet) override;

    virtual cGate *getRegistrationForwardingGate(cGate *gate) override;
};

} // namespace inet

#endif

