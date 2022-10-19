//
// Copyright (C) 2022 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/networklayer/contract/netfilter/INetfilterCompatibleNetfilterHookManagerBase.h"

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/networklayer/contract/netfilter/NetfilterHookDefs_m.h"

namespace inet {
namespace NetfilterHook {

NetfilterResult INetfilterCompatibleNetfilterHookManagerBase::mapResult(INetfilter::IHook::Result r)
{
    switch (r) {
        case INetfilter::IHook::Result::ACCEPT: return NetfilterResult::ACCEPT;
        case INetfilter::IHook::Result::DROP: return NetfilterResult::DROP;
        case INetfilter::IHook::Result::QUEUE: return NetfilterResult::QUEUE;
        //case INetfilter::IHook::Result::REPEAT: return NetfilterResult::REPEAT;
        case INetfilter::IHook::Result::STOLEN: return NetfilterResult::STOLEN;
        //case INetfilter::IHook::Result::STOP: return NetfilterResult::STOP;
        default: throw cRuntimeError("Unknown result value");
    }
}

void INetfilterCompatibleNetfilterHookManagerBase::registerHook(int priority, IHook *hook)
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
    h.prerouting = new NetfilterHandler([=](Packet* packet) { return mapResult(hook->datagramPreRoutingHook(packet)); });
    h.localIn = new NetfilterHandler([=](Packet* packet) { return mapResult(hook->datagramLocalInHook(packet)); });
    h.forward = new NetfilterHandler([=](Packet* packet) { return mapResult(hook->datagramForwardHook(packet)); });
    h.postrouting = new NetfilterHandler([=](Packet* packet) { return mapResult(hook->datagramPostRoutingHook(packet)); });
    h.localOut = new NetfilterHandler([=](Packet* packet) { return mapResult(hook->datagramLocalOutHook(packet)); });

    registerNetfilterHandler(NetfilterType::PREROUTING, h.priority, h.prerouting);
    registerNetfilterHandler(NetfilterType::LOCALIN, h.priority, h.localIn);
    registerNetfilterHandler(NetfilterType::FORWARD, h.priority, h.forward);
    registerNetfilterHandler(NetfilterType::POSTROUTING, h.priority, h.postrouting);
    registerNetfilterHandler(NetfilterType::LOCALOUT, h.priority, h.localOut);

    hookInfo.insert(std::pair<IHook*, IHookHandlers>(hook, h));
}

void INetfilterCompatibleNetfilterHookManagerBase::unregisterHook(IHook *hook)
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
    unregisterNetfilterHandler(NetfilterType::PREROUTING, h.priority, h.prerouting);
    unregisterNetfilterHandler(NetfilterType::LOCALIN, h.priority, h.localIn);
    unregisterNetfilterHandler(NetfilterType::FORWARD, h.priority, h.forward);
    unregisterNetfilterHandler(NetfilterType::POSTROUTING, h.priority, h.postrouting);
    unregisterNetfilterHandler(NetfilterType::LOCALOUT, h.priority, h.localOut);
    hookInfo.erase(it);
}

void INetfilterCompatibleNetfilterHookManagerBase::dropQueuedDatagram(const Packet *datagram)
{
    reinjectDatagram(const_cast<Packet *>(datagram), NetfilterResult::DROP);
}

void INetfilterCompatibleNetfilterHookManagerBase::reinjectQueuedDatagram(const Packet *datagram)
{
    reinjectDatagram(const_cast<Packet *>(datagram), NetfilterResult::ACCEPT);
}

} // namespace NetfilterHook
} // namespace inet
