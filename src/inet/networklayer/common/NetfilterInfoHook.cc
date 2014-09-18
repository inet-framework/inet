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

#include <vector>
#include <string>

#include "inet/common/INETDefs.h"

#include "inet/common/ModuleAccess.h"
#include "inet/networklayer/common/InterfaceEntry.h"
#include "inet/networklayer/common/INetfilter.h"
#include "inet/networklayer/contract/INetworkDatagram.h"

namespace inet {

class INET_API NetfilterInfoHook : public cSimpleModule, public INetfilter::IHook
{
  protected:
    INetfilter *netfilter;

  protected:
    virtual void initialize(int stage);
    virtual int numInitStages() const { return NUM_INIT_STAGES; }
    virtual void handleMessage(cMessage *msg);
    virtual void finish();

  public:
    /**
     * called before a packet arriving from the network is routed
     */
    virtual Result datagramPreRoutingHook(INetworkDatagram *datagram, const InterfaceEntry *inIE, const InterfaceEntry *& outIE, L3Address& nextHopAddr);

    /**
     * called before a packet arriving from the network is delivered via the network
     */
    virtual Result datagramForwardHook(INetworkDatagram *datagram, const InterfaceEntry *inIE, const InterfaceEntry *& outIE, L3Address& nextHopAddr);

    /**
     * called before a packet is delivered via the network
     */
    virtual Result datagramPostRoutingHook(INetworkDatagram *datagram, const InterfaceEntry *inIE, const InterfaceEntry *& outIE, L3Address& nextHopAddr);

    /**
     * called before a packet arriving from the network is delivered locally
     */
    virtual Result datagramLocalInHook(INetworkDatagram *datagram, const InterfaceEntry *inIE);

    /**
     * called before a packet arriving locally is delivered
     */
    virtual Result datagramLocalOutHook(INetworkDatagram *datagram, const InterfaceEntry *& outIE, L3Address& nextHopAddr);
};

Define_Module(NetfilterInfoHook);

void NetfilterInfoHook::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_NETWORK_LAYER) {
        netfilter = check_and_cast<INetfilter *>(findModuleWhereverInNode("ip", this));
        netfilter->registerHook(0, this);
    }
}

void NetfilterInfoHook::handleMessage(cMessage *msg)
{
    throw cRuntimeError("This module can not receive messages");
}

INetfilter::IHook::Result NetfilterInfoHook::datagramPreRoutingHook(INetworkDatagram *datagram, const InterfaceEntry *inIE, const InterfaceEntry *& outIE, L3Address& nextHopAddr)
{
    EV_INFO << "HOOK " << getFullPath() << ": PREROUTING packet=" << dynamic_cast<cObject *>(datagram)->getName()
            << " inIE=" << (inIE ? inIE->getName() : "NULL")
            << endl;
    return INetfilter::IHook::ACCEPT;
}

INetfilter::IHook::Result NetfilterInfoHook::datagramForwardHook(INetworkDatagram *datagram, const InterfaceEntry *inIE, const InterfaceEntry *& outIE, L3Address& nextHopAddr)
{
    EV_INFO << "HOOK " << getFullPath() << ": FORWARD: packet=" << dynamic_cast<cObject *>(datagram)->getName()
            << " inIE=" << (inIE ? inIE->getName() : "NULL")
            << " outIE=" << (outIE ? outIE->getName() : "NULL")
            << " nextHop=" << nextHopAddr
            << endl;
    return INetfilter::IHook::ACCEPT;
}

INetfilter::IHook::Result NetfilterInfoHook::datagramPostRoutingHook(INetworkDatagram *datagram, const InterfaceEntry *inIE, const InterfaceEntry *& outIE, L3Address& nextHopAddr)
{
    EV_INFO << "HOOK " << getFullPath() << ": POSTROUTING packet=" << dynamic_cast<cObject *>(datagram)->getName()
            << " inIE=" << (inIE ? inIE->getName() : "NULL")
            << " outIE=" << (outIE ? outIE->getName() : "NULL")
            << " nextHop=" << nextHopAddr
            << endl;
    return INetfilter::IHook::ACCEPT;
}

INetfilter::IHook::Result NetfilterInfoHook::datagramLocalInHook(INetworkDatagram *datagram, const InterfaceEntry *inIE)
{
    EV_INFO << "HOOK " << getFullPath() << ": LOCAL IN: packet=" << dynamic_cast<cObject *>(datagram)->getName()
            << " inIE=" << (inIE ? inIE->getName() : "NULL")
            << endl;
    return INetfilter::IHook::ACCEPT;
}

INetfilter::IHook::Result NetfilterInfoHook::datagramLocalOutHook(INetworkDatagram *datagram, const InterfaceEntry *& outIE, L3Address& nextHopAddr)
{
    EV_INFO << "HOOK " << getFullPath() << ": LOCAL OUT: packet=" << dynamic_cast<cObject *>(datagram)->getName()
            << " outIE=" << (outIE ? outIE->getName() : "NULL")
            << endl;
    return INetfilter::IHook::ACCEPT;
}

void NetfilterInfoHook::finish()
{
    netfilter->unregisterHook(0, this);
}

} // namespace inet

