//
// Copyright (C) 2022 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/networklayer/ipv4layer/ipv4_modular/Ipv4HookManager.h"

#include "inet/networklayer/contract/netfilter/NetfilterQueuedDatagramTag_m.h"

namespace inet {

Define_Module(Ipv4HookManager);

void Ipv4HookManager::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        hook[NetfilterHook::NetfilterType::PREROUTING] = check_and_cast<Ipv4NetfilterHook *>(getModuleByPath("^.preroutingHook"));
        hook[NetfilterHook::NetfilterType::LOCALIN] = check_and_cast<Ipv4NetfilterHook *>(getModuleByPath("^.inputHook"));
        hook[NetfilterHook::NetfilterType::FORWARD] = check_and_cast<Ipv4NetfilterHook *>(getModuleByPath("^.forwardHook"));
        hook[NetfilterHook::NetfilterType::POSTROUTING] = check_and_cast<Ipv4NetfilterHook *>(getModuleByPath("^.postroutingHook"));
        hook[NetfilterHook::NetfilterType::LOCALOUT] = check_and_cast<Ipv4NetfilterHook *>(getModuleByPath("^.outputHook"));
    }
}

void Ipv4HookManager::registerNetfilterHandler(NetfilterHook::NetfilterType type, int priority, NetfilterHook::NetfilterHandler *handler)
{
    Enter_Method(__FUNCTION__);

    ASSERT(type >= 0 && type < NetfilterHook::NetfilterType::__NUM_HOOK_TYPES);
    hook[type]->registerNetfilterHandler(priority, handler);
}

void Ipv4HookManager::unregisterNetfilterHandler(NetfilterHook::NetfilterType type, int priority, NetfilterHook::NetfilterHandler *handler)
{
    Enter_Method(__FUNCTION__);

    ASSERT(type >= 0 && type < NetfilterHook::NetfilterType::__NUM_HOOK_TYPES);
    hook[type]->unregisterNetfilterHandler(priority, handler);
}

void Ipv4HookManager::reinjectDatagram(Packet *datagram, NetfilterHook::NetfilterResult action)
{
    Enter_Method(__FUNCTION__);

    auto tag = datagram->getTag<NetfilterHook::NetfilterQueuedDatagramTag>();
    for (int hookPos = NetfilterHook::NetfilterType::__FIRST_HOOK; hookPos < NetfilterHook::NetfilterType::__NUM_HOOK_TYPES; ++hookPos) {
        if (hook[hookPos]->getId() == tag->getHookId()) {
            hook[hookPos]->reinjectDatagram(datagram, action);
            return;
        }
    }
    throw cRuntimeError("Hook not found for ID=%d", tag->getHookId());
}

} // namespace inet
