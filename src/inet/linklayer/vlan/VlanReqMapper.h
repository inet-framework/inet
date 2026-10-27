//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_VLANREQMAPPER_H
#define __INET_VLANREQMAPPER_H

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ModuleRefByPar.h"
#include "inet/linklayer/vlan/VlanIdMaps.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/queueing/base/PacketFlowBase.h"

namespace inet {

using namespace inet::queueing;

class INET_API VlanReqMapper : public PacketFlowBase, public TransparentProtocolRegistrationListener
{
  protected:
    const Protocol *protocol = nullptr;
    ModuleRefByPar<IInterfaceTable> interfaceTable;
    std::map<std::string, std::map<int, int>> mappedVlanIds; // interface name or "*" -> requested VLAN id -> mapped VLAN id

  protected:
    virtual void initialize(int stage) override;
    virtual void processPacket(Packet *packet) override;

    virtual cGate *getRegistrationForwardingGate(cGate *gate) override;
};

} // namespace inet

#endif

