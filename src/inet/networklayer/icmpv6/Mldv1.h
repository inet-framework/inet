//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_MLDV1_H
#define __INET_MLDV1_H

#include "inet/common/ModuleRefByPar.h"
#include "inet/common/SimpleModule.h"
#include "inet/common/checksum/ChecksumMode_m.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/lifecycle/OperationalBase.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/networklayer/contract/ipv6/Ipv6Address.h"

namespace inet {

class IInterfaceTable;
class Ipv6RoutingTable;

class INET_API Mldv1 : public OperationalBase
{
  protected:
    ModuleRefByPar<IInterfaceTable> ift;
    ModuleRefByPar<Ipv6RoutingTable> rt;

    bool enabled = true;
    ChecksumMode checksumMode = CHECKSUM_MODE_UNDEFINED;
    bool sendTestMessage = false;
    cMessage *testTimer = nullptr;

  protected:
    virtual bool isInitializeStage(int stage) const override { return stage == INITSTAGE_NETWORK_LAYER; }
    virtual bool isModuleStartStage(int stage) const override { return stage == ModuleStartOperation::STAGE_NETWORK_LAYER; }
    virtual bool isModuleStopStage(int stage) const override { return stage == ModuleStopOperation::STAGE_NETWORK_LAYER; }
    virtual void handleStartOperation(LifecycleOperation *operation) override {}
    virtual void handleStopOperation(LifecycleOperation *operation) override {}
    virtual void handleCrashOperation(LifecycleOperation *operation) override {}

    virtual ~Mldv1() { cancelAndDelete(testTimer); }

    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;

    virtual void processMldMessage(Packet *packet);
    virtual void sendToIPv6(Packet *msg, NetworkInterface *ie, const Ipv6Address& dest);
};

} // namespace inet

#endif // ifndef __INET_MLDV1_H
