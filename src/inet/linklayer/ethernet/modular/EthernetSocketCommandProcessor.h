//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ETHERNETSOCKETCOMMANDPROCESSOR_H
#define __INET_ETHERNETSOCKETCOMMANDPROCESSOR_H

#include "inet/common/ModuleRefByPar.h"
#include "inet/common/packet/Message.h"
#include "inet/linklayer/ethernet/contract/IEthernet.h"
#include "inet/linklayer/ethernet/modular/EthernetSocketTable.h"
#include "inet/queueing/base/PacketFlowBase.h"

namespace inet {

class INET_API EthernetSocketCommandProcessor : public queueing::PacketFlowBase, public IEthernet
{
  protected:
    ModuleRefByPar<EthernetSocketTable> socketTable;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;
    virtual void handleCommand(Request *request);
    virtual void processPacket(Packet *packet) override {}

  public:
    virtual void bind(int socketId, int interfaceId, const MacAddress& localAddress, const MacAddress& remoteAddress, const Protocol *protocol, bool steal) override;
};

} // namespace inet

#endif

