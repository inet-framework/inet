//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE8022LLCSOCKETCOMMANDPROCESSOR_H
#define __INET_IEEE8022LLCSOCKETCOMMANDPROCESSOR_H

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ModuleRefByPar.h"
#include "inet/common/packet/Message.h"
#include "inet/linklayer/ieee8022/Ieee8022LlcSocketTable.h"
#include "inet/queueing/base/PacketFlowBase.h"

namespace inet {

class INET_API Ieee8022LlcSocketCommandProcessor : public queueing::PacketFlowBase, public TransparentProtocolRegistrationListener
{
  protected:
    ModuleRefByPar<Ieee8022LlcSocketTable> socketTable;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;
    virtual void handleCommand(Request *request);
    virtual void processPacket(Packet *packet) override {}

    virtual cGate *getRegistrationForwardingGate(cGate *gate) override;
};

} // namespace inet

#endif

