//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_RELAYINTERFACELEARNER_H
#define __INET_RELAYINTERFACELEARNER_H

#include "inet/common/ModuleRefByPar.h"
#include "inet/linklayer/ethernet/contract/IMacForwardingTable.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/queueing/base/PacketFlowBase.h"

namespace inet {

using namespace inet::queueing;

class INET_API RelayInterfaceLearner : public PacketFlowBase
{
  protected:
    ModuleRefByPar<IInterfaceTable> interfaceTable;
    ModuleRefByPar<IMacForwardingTable> macForwardingTable;

  protected:
    virtual void initialize(int stage) override;

  protected:
    virtual void processPacket(Packet *packet) override;
};

} // namespace inet

#endif

