//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "NetfilterHookProbe.h"

#include "inet/common/packet/Packet.h"

namespace inet {

Define_Module(NetfilterHookProbe);

INetfilter::IHook::Result NetfilterHookProbe::parseVerdict(const char *s)
{
    if (!strcmp(s, "ACCEPT")) return IHook::ACCEPT;
    if (!strcmp(s, "DROP")) return IHook::DROP;
    if (!strcmp(s, "QUEUE")) return IHook::QUEUE;
    throw cRuntimeError("Unknown verdict '%s' (expected ACCEPT, DROP or QUEUE)", s);
}

void NetfilterHookProbe::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        reinjectDelay = par("reinjectDelay").doubleValue();
        preRoutingVerdict = parseVerdict(par("preRouting").stringValue());
        localInVerdict = parseVerdict(par("localIn").stringValue());
        forwardVerdict = parseVerdict(par("forward").stringValue());
        postRoutingVerdict = parseVerdict(par("postRouting").stringValue());
        localOutVerdict = parseVerdict(par("localOut").stringValue());

        netfilter = check_and_cast<INetfilter *>(getModuleByPath(par("networkProtocolModule").stringValue()));
        netfilter->registerHook(0, this);

        WATCH(preRoutingCount);
        WATCH(localInCount);
        WATCH(forwardCount);
        WATCH(postRoutingCount);
        WATCH(localOutCount);
        WATCH(reinjectCount);
        WATCH(dropCount);
    }
}

INetfilter::IHook::Result NetfilterHookProbe::process(Packet *datagram, IHook::Result verdict, const char *hookName, long& counter)
{
    Enter_Method_Silent();
    counter++;
    const char *verdictName = verdict == IHook::ACCEPT ? "ACCEPT" : verdict == IHook::DROP ? "DROP" : "QUEUE";
    EV_INFO << "PROBE " << getFullPath() << " " << hookName << " -> " << verdictName << ", packet: " << datagram->getName() << std::endl;
    if (verdict == IHook::QUEUE) {
        cMessage *selfMsg = new cMessage("reinject");
        selfMsg->setContextPointer(datagram);
        scheduleAt(simTime() + reinjectDelay, selfMsg);
    }
    else if (verdict == IHook::DROP)
        dropCount++;
    return verdict;
}

void NetfilterHookProbe::handleMessage(cMessage *msg)
{
    ASSERT(msg->isSelfMessage());
    Packet *datagram = static_cast<Packet *>(msg->getContextPointer());
    delete msg;
    reinjectCount++;
    EV_INFO << "PROBE " << getFullPath() << " reinjecting packet: " << datagram->getName() << std::endl;
    netfilter->reinjectQueuedDatagram(datagram);
}

void NetfilterHookProbe::finish()
{
    EV_INFO << "PROBE-SUMMARY " << getFullPath()
            << " preRouting=" << preRoutingCount
            << " localIn=" << localInCount
            << " forward=" << forwardCount
            << " postRouting=" << postRoutingCount
            << " localOut=" << localOutCount
            << " reinject=" << reinjectCount
            << " drop=" << dropCount << std::endl;
    recordScalar("preRoutingCount", preRoutingCount);
    recordScalar("localInCount", localInCount);
    recordScalar("forwardCount", forwardCount);
    recordScalar("postRoutingCount", postRoutingCount);
    recordScalar("localOutCount", localOutCount);
    recordScalar("reinjectCount", reinjectCount);
    recordScalar("dropCount", dropCount);
}

INetfilter::IHook::Result NetfilterHookProbe::datagramPreRoutingHook(Packet *datagram) { return process(datagram, preRoutingVerdict, "PREROUTING", preRoutingCount); }
INetfilter::IHook::Result NetfilterHookProbe::datagramForwardHook(Packet *datagram) { return process(datagram, forwardVerdict, "FORWARD", forwardCount); }
INetfilter::IHook::Result NetfilterHookProbe::datagramPostRoutingHook(Packet *datagram) { return process(datagram, postRoutingVerdict, "POSTROUTING", postRoutingCount); }
INetfilter::IHook::Result NetfilterHookProbe::datagramLocalInHook(Packet *datagram) { return process(datagram, localInVerdict, "LOCALIN", localInCount); }
INetfilter::IHook::Result NetfilterHookProbe::datagramLocalOutHook(Packet *datagram) { return process(datagram, localOutVerdict, "LOCALOUT", localOutCount); }

} // namespace inet
