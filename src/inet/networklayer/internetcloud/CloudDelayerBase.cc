//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/networklayer/internetcloud/CloudDelayerBase.h"

#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/ipv4/Ipv4.h"

namespace inet {

Define_Module(CloudDelayerBase);

#define SRCPAR    "incomingInterfaceID"

CloudDelayerBase::CloudDelayerBase()
{
}

CloudDelayerBase::~CloudDelayerBase()
{
}

void CloudDelayerBase::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        networkProtocol.reference(this, "networkProtocolModule", true);
    }
    else if (stage == INITSTAGE_NETWORK_LAYER) {
        networkProtocol->registerHook(0, this);
    }
}

void CloudDelayerBase::finish()
{
    if (isRegisteredHook(networkProtocol.get()))
        networkProtocol->unregisterHook(this);
}

void CloudDelayerBase::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        Packet *context = (Packet *)msg->getContextPointer();
        delete msg;
        networkProtocol->reinjectQueuedDatagram(context);
    }
    else
        throw cRuntimeError("This module does not handle incoming messages");
}

void CloudDelayerBase::calculateDropAndDelay(const cMessage *msg, int srcID, int destID, bool& outDrop, simtime_t& outDelay)
{
    outDrop = false;
    outDelay = SIMTIME_ZERO;
}

INetfilter::IHook::Result CloudDelayerBase::datagramPreRoutingHook(Packet *datagram)
{
    return INetfilter::IHook::ACCEPT;
}

INetfilter::IHook::Result CloudDelayerBase::datagramForwardHook(Packet *datagram)
{
    Enter_Method("datagramForwardHook");

    auto ifInd = datagram->getTag<InterfaceInd>();
    int srcID = ifInd ? ifInd->getInterfaceId() : -1;
    int destID = datagram->getTag<InterfaceReq>()->getInterfaceId();

    simtime_t propDelay;
    bool isDrop;
    calculateDropAndDelay(datagram, srcID, destID, isDrop, propDelay);
    if (isDrop) {
        // TODO emit?
        EV_INFO << "Message " << datagram->str() << " dropped in cloud.\n";
        return INetfilter::IHook::DROP;
    }

    if (propDelay > SIMTIME_ZERO) {
        // TODO emit?
        EV_INFO << "Message " << datagram->str() << " delayed with " << propDelay * 1000.0 << "ms in cloud.\n";
        cMessage *selfmsg = new cMessage("Delay");
        selfmsg->setContextPointer(datagram); // datagram owned by INetfilter module (Ipv4, Ipv6, ...)
        scheduleAfter(propDelay, selfmsg);
        return INetfilter::IHook::QUEUE;
    }
    return INetfilter::IHook::ACCEPT;
}

INetfilter::IHook::Result CloudDelayerBase::datagramPostRoutingHook(Packet *datagram)
{
    return INetfilter::IHook::ACCEPT;
}

INetfilter::IHook::Result CloudDelayerBase::datagramLocalInHook(Packet *datagram)
{
    return INetfilter::IHook::ACCEPT;
}

INetfilter::IHook::Result CloudDelayerBase::datagramLocalOutHook(Packet *datagram)
{
    return INetfilter::IHook::ACCEPT;
}

} // namespace inet

