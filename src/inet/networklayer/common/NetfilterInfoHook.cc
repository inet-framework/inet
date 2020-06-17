//
// This library is free software, you can redistribute it
// and/or modify
// it under  the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// Copyright 2004 Andras Varga
//

#include "inet/common/INETDefs.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/packet/Packet.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/InterfaceEntry.h"
#include "inet/networklayer/common/NextHopAddressTag_m.h"
#include "inet/networklayer/contract/INetfilter.h"

namespace inet {

class INET_API NetfilterInfoHook : public cSimpleModule, public NetfilterBase::HookBase
{
  protected:
    INetfilter *netfilter;

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
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_NETWORK_LAYER) {
        netfilter = getModuleFromPar<INetfilter>(par("networkProtocolModule"), this);
        netfilter->registerHook(0, this);
    }
}

void NetfilterInfoHook::handleMessage(cMessage *msg)
{
    throw cRuntimeError("This module can not receive messages");
}

INetfilter::IHook::Result NetfilterInfoHook::datagramPreRoutingHook(Packet *datagram)
{
    EV_INFO << "HOOK " << getFullPath() << ": PREROUTING packet=" << datagram->getName()
            // TODO: find out interface name
            << " inIE=" << std::to_string(datagram->getTag<InterfaceInd>()->getInterfaceId())
            << endl;
    return INetfilter::IHook::ACCEPT;
}

INetfilter::IHook::Result NetfilterInfoHook::datagramForwardHook(Packet *datagram)
{
    EV_INFO << "HOOK " << getFullPath() << ": FORWARD: packet=" << datagram->getName()
            << " inIE=" << std::to_string(datagram->getTag<InterfaceInd>()->getInterfaceId())
            << " outIE=" << std::to_string(datagram->getTag<InterfaceReq>()->getInterfaceId())
            << " nextHop=" << datagram->getTag<NextHopAddressReq>()->getNextHopAddress()
            << endl;
    return INetfilter::IHook::ACCEPT;
}

INetfilter::IHook::Result NetfilterInfoHook::datagramPostRoutingHook(Packet *datagram)
{
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
    EV_INFO << "HOOK " << getFullPath() << ": LOCAL IN: packet=" << datagram->getName()
            << " inIE=" << datagram->getTag<InterfaceInd>()->getInterfaceId()
            << endl;
    return INetfilter::IHook::ACCEPT;
}

INetfilter::IHook::Result NetfilterInfoHook::datagramLocalOutHook(Packet *datagram)
{
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

