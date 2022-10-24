//
// Copyright (C) 2022 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/networklayer/ipv4modular/INetfilterCompatibleIpv4HookManagerBase.h"

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/networklayer/ipv4modular/IIpv4HookManager_m.h"

namespace inet {

Ipv4Hook::NetfilterResult INetfilterCompatibleIpv4HookManagerBase::mapResult(INetfilter::IHook::Result r)
{
    switch (r) {
        case INetfilter::IHook::Result::ACCEPT: return Ipv4Hook::NetfilterResult::ACCEPT;
        case INetfilter::IHook::Result::DROP: return Ipv4Hook::NetfilterResult::DROP;
        case INetfilter::IHook::Result::QUEUE: return Ipv4Hook::NetfilterResult::QUEUE;
        //case INetfilter::IHook::Result::REPEAT: return Ipv4Hook::NetfilterResult::REPEAT;
        case INetfilter::IHook::Result::STOLEN: return Ipv4Hook::NetfilterResult::STOLEN;
        //case INetfilter::IHook::Result::STOP: return Ipv4Hook::NetfilterResult::STOP;
        default: throw cRuntimeError("Unknown result value");
    }
}

void INetfilterCompatibleIpv4HookManagerBase::registerHook(int priority, IHook *hook)
{
//    Enter_Method(__FUNCTION__);
    omnetpp::cMethodCallContextSwitcher __ctx(check_and_cast<cComponent *>(this)); __ctx.methodCall(__FUNCTION__);
#ifdef INET_WITH_SELFDOC
    __Enter_Method_SelfDoc(__FUNCTION__);
#endif

    if (hookInfo.find(hook) != hookInfo.end())
        throw cRuntimeError("Usage error: the hook already registered.");

    IHookHandlers h;

    h.priority = priority;
    h.prerouting = new Ipv4Hook::NetfilterHandler([=](Packet* packet) { return mapResult(hook->datagramPreRoutingHook(packet)); });
    h.localIn = new Ipv4Hook::NetfilterHandler([=](Packet* packet) { return mapResult(hook->datagramLocalInHook(packet)); });
    h.forward = new Ipv4Hook::NetfilterHandler([=](Packet* packet) { return mapResult(hook->datagramForwardHook(packet)); });
    h.postrouting = new Ipv4Hook::NetfilterHandler([=](Packet* packet) { return mapResult(hook->datagramPostRoutingHook(packet)); });
    h.localOut = new Ipv4Hook::NetfilterHandler([=](Packet* packet) { return mapResult(hook->datagramLocalOutHook(packet)); });

    registerNetfilterHandler(Ipv4Hook::NetfilterType::PREROUTING, h.priority, h.prerouting);
    registerNetfilterHandler(Ipv4Hook::NetfilterType::LOCALIN, h.priority, h.localIn);
    registerNetfilterHandler(Ipv4Hook::NetfilterType::FORWARD, h.priority, h.forward);
    registerNetfilterHandler(Ipv4Hook::NetfilterType::POSTROUTING, h.priority, h.postrouting);
    registerNetfilterHandler(Ipv4Hook::NetfilterType::LOCALOUT, h.priority, h.localOut);

    hookInfo.insert(std::pair<IHook*, IHookHandlers>(hook, h));
}

void INetfilterCompatibleIpv4HookManagerBase::unregisterHook(IHook *hook)
{
//    Enter_Method(__FUNCTION__);
    omnetpp::cMethodCallContextSwitcher __ctx(check_and_cast<cComponent *>(this)); __ctx.methodCall(__FUNCTION__);
#ifdef INET_WITH_SELFDOC
    __Enter_Method_SelfDoc(__FUNCTION__);
#endif

    auto it = hookInfo.find(hook);
    if (it == hookInfo.end())
        throw cRuntimeError("Usage error: the hook not found in list of registered hooks.");

    const auto& h = it->second;
    unregisterNetfilterHandler(Ipv4Hook::NetfilterType::PREROUTING, h.priority, h.prerouting);
    unregisterNetfilterHandler(Ipv4Hook::NetfilterType::LOCALIN, h.priority, h.localIn);
    unregisterNetfilterHandler(Ipv4Hook::NetfilterType::FORWARD, h.priority, h.forward);
    unregisterNetfilterHandler(Ipv4Hook::NetfilterType::POSTROUTING, h.priority, h.postrouting);
    unregisterNetfilterHandler(Ipv4Hook::NetfilterType::LOCALOUT, h.priority, h.localOut);
    hookInfo.erase(it);
}

void INetfilterCompatibleIpv4HookManagerBase::dropQueuedDatagram(const Packet *datagram)
{
    reinjectDatagram(const_cast<Packet *>(datagram), Ipv4Hook::NetfilterResult::DROP);
}

void INetfilterCompatibleIpv4HookManagerBase::reinjectQueuedDatagram(const Packet *datagram)
{
    reinjectDatagram(const_cast<Packet *>(datagram), Ipv4Hook::NetfilterResult::ACCEPT);
}

} // namespace inet
