//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/common/SimpleModule.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/contract/INetfilter.h"
#include "inet/networklayer/ipv6/Ipv6Header_m.h"

namespace inet {

class InnerDatagramDropper : public SimpleModule, public NetfilterBase::HookBase
{
  protected:
    virtual void initialize(int stage) override
    {
        SimpleModule::initialize(stage);
        if (stage == INITSTAGE_LOCAL) {
            auto netfilter = check_and_cast<INetfilter *>(getModuleByPath(par("networkProtocolModule").stringValue()));
            netfilter->registerHook(0, this);
        }
    }
    virtual void handleMessage(cMessage *msg) override { throw cRuntimeError("This module can not receive messages"); }

  public:
    virtual Result datagramPreRoutingHook(Packet *datagram) override
    {
        Enter_Method_Silent();
        if (datagram->peekAtFront<Ipv6Header>()->getProtocolId() != IP_PROT_UDP)
            return ACCEPT;
        EV_INFO << "Dropping " << datagram->getName() << EV_ENDL;
        return DROP;
    }
    virtual Result datagramForwardHook(Packet *datagram) override { return ACCEPT; }
    virtual Result datagramPostRoutingHook(Packet *datagram) override { return ACCEPT; }
    virtual Result datagramLocalInHook(Packet *datagram) override { return ACCEPT; }
    virtual Result datagramLocalOutHook(Packet *datagram) override { return ACCEPT; }
};

Define_Module(InnerDatagramDropper);

} // namespace inet
