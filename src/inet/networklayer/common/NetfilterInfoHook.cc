//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/SimpleModule.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ModuleRefByPar.h"
#include "inet/common/packet/Packet.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/networklayer/common/NextHopAddressTag_m.h"
#include "inet/networklayer/contract/INetfilter.h"

namespace inet {

class INET_API NetfilterInfoHook : public SimpleModule, public NetfilterBase::HookBase
{
  protected:
    ModuleRefByPar<INetfilter> netfilter;

  protected:
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;

  public:
    /**
     * called before a packet arriving from the network is routed
     */
    virtual Result datagramPreRoutingHook(Packet *datagram) override;

    /**
     * called before a packet arriving from the network is delivered via the network
     */
    virtual Result datagramForwardHook(Packet *datagram) override;

    /**
     * called before a packet is delivered via the network
     */
    virtual Result datagramPostRoutingHook(Packet *datagram) override;

    /**
     * called before a packet arriving from the network is delivered locally
     */
    virtual Result datagramLocalInHook(Packet *datagram) override;

    /**
     * called before a packet arriving locally is delivered
     */
    virtual Result datagramLocalOutHook(Packet *datagram) override;
};

Define_Module(NetfilterInfoHook);

void NetfilterInfoHook::initialize(int stage)
{
    SimpleModule::initialize(stage);

    if (stage == INITSTAGE_NETWORK_LAYER) {
        netfilter.reference(this, "networkProtocolModule", true);
        netfilter->registerHook(0, this);
    }
}

void NetfilterInfoHook::handleMessage(cMessage *msg)
{
    throw cRuntimeError("This module can not receive messages");
}

INetfilter::IHook::Result NetfilterInfoHook::datagramPreRoutingHook(Packet *datagram)
{
    Enter_Method("datagramPreRoutingHook");

    EV_INFO << "HOOK " << getFullPath() << ": PREROUTING packet=" << datagram->getName()
        // TODO find out interface name
            << " inIE=" << std::to_string(datagram->getTag<InterfaceInd>()->getInterfaceId())
            << endl;
    return INetfilter::IHook::ACCEPT;
}

INetfilter::IHook::Result NetfilterInfoHook::datagramForwardHook(Packet *datagram)
{
    Enter_Method("datagramForwardHook");

    EV_INFO << "HOOK " << getFullPath() << ": FORWARD: packet=" << datagram->getName()
            << " inIE=" << std::to_string(datagram->getTag<InterfaceInd>()->getInterfaceId())
            << " outIE=" << std::to_string(datagram->getTag<InterfaceReq>()->getInterfaceId())
            << " nextHop=" << datagram->getTag<NextHopAddressReq>()->getNextHopAddress()
            << endl;
    return INetfilter::IHook::ACCEPT;
}

INetfilter::IHook::Result NetfilterInfoHook::datagramPostRoutingHook(Packet *datagram)
{
    Enter_Method("datagramPostRoutingHook");

    const auto& interfaceInd = datagram->findTag<InterfaceInd>();
    EV_INFO << "HOOK " << getFullPath() << ": POSTROUTING packet=" << datagram->getName()
            << " inIE=" << (interfaceInd ? std::to_string(interfaceInd->getInterfaceId()) : "undefined")
            << " outIE=" << std::to_string(datagram->getTag<InterfaceReq>()->getInterfaceId())
            << " nextHop=" << datagram->getTag<NextHopAddressReq>()->getNextHopAddress()
            << endl;
    return INetfilter::IHook::ACCEPT;
}

INetfilter::IHook::Result NetfilterInfoHook::datagramLocalInHook(Packet *datagram)
{
    Enter_Method("datagramLocalInHook");

    EV_INFO << "HOOK " << getFullPath() << ": LOCAL IN: packet=" << datagram->getName()
            << " inIE=" << datagram->getTag<InterfaceInd>()->getInterfaceId()
            << endl;
    return INetfilter::IHook::ACCEPT;
}

INetfilter::IHook::Result NetfilterInfoHook::datagramLocalOutHook(Packet *datagram)
{
    Enter_Method("datagramLocalOutHook");

    const auto& interfaceReq = datagram->findTag<InterfaceReq>();
    EV_INFO << "HOOK " << getFullPath() << ": LOCAL OUT: packet=" << datagram->getName()
            << " outIE=" << (interfaceReq ? std::to_string(interfaceReq->getInterfaceId()) : "undefined")
            << endl;
    return INetfilter::IHook::ACCEPT;
}

void NetfilterInfoHook::finish()
{
    if (isRegisteredHook(netfilter))
        netfilter->unregisterHook(this);
}

} // namespace inet

