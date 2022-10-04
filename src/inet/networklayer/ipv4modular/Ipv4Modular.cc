//
// Copyright (C) 2022 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/networklayer/ipv4modular/Ipv4Modular.h"

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/networklayer/ipv4modular/IIpv4HookManager_m.h"

namespace inet {

Define_Module(Ipv4Modular);

void Ipv4Modular::initialize(int stage)
{
    cModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        hookManager = check_and_cast_nullable<IIpv4HookManager *>(findModuleByPath(".hookManager"));
    }
    else if (stage == INITSTAGE_NETWORK_LAYER) {
        registerService(Protocol::ipv4, gate("transportIn"), gate("transportOut"));
        registerProtocol(Protocol::ipv4, gate("queueOut"), gate("queueIn"));
    }
}

static Ipv4Hook::NetfilterResult mapResult(INetfilter::IHook::Result r)
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

void Ipv4Modular::registerHook(int priority, IHook *hook)
{
    Enter_Method(__FUNCTION__);

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

void Ipv4Modular::unregisterHook(IHook *hook)
{
    Enter_Method(__FUNCTION__);

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

void Ipv4Modular::chkHookManager()
{
    if (!hookManager)
        throw cRuntimeError("No hook manager, hooks not supported");
}

void Ipv4Modular::dropQueuedDatagram(const Packet *datagram)
{
    Enter_Method(__FUNCTION__);

    chkHookManager();
    hookManager->reinjectQueuedDatagram(const_cast<Packet *>(datagram), Ipv4Hook::NetfilterResult::DROP);
}

void Ipv4Modular::reinjectQueuedDatagram(const Packet *datagram)
{
    Enter_Method(__FUNCTION__);

    reinjectQueuedDatagram(const_cast<Packet *>(datagram), Ipv4Hook::NetfilterResult::ACCEPT);
}

void Ipv4Modular::reinjectQueuedDatagram(Packet *datagram, Ipv4Hook::NetfilterResult action)
{
    Enter_Method(__FUNCTION__);

    chkHookManager();
    hookManager->reinjectQueuedDatagram(datagram, action);
}

void Ipv4Modular::registerNetfilterHandler(Ipv4Hook::NetfilterType type, int priority, Ipv4Hook::NetfilterHandler *handler)
{
    Enter_Method(__FUNCTION__);

    chkHookManager();
    hookManager->registerNetfilterHandler(type, priority, handler);
}

void Ipv4Modular::unregisterNetfilterHandler(Ipv4Hook::NetfilterType type, int priority, Ipv4Hook::NetfilterHandler *handler)
{
    Enter_Method(__FUNCTION__);

    chkHookManager();
    hookManager->unregisterNetfilterHandler(type, priority, handler);
}

} // namespace inet
