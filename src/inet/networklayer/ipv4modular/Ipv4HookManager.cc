//
// Copyright (C) 2022 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/networklayer/ipv4modular/Ipv4HookManager.h"

#include "inet/networklayer/ipv4modular/Ipv4QueuedDatagramTag_m.h"

namespace inet {

Define_Module(Ipv4HookManager);

void Ipv4HookManager::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        hook[Ipv4Hook::NetfilterType::PREROUTING] = check_and_cast<Ipv4NetfilterHook *>(getModuleByPath("^.preroutingHook"));
        hook[Ipv4Hook::NetfilterType::LOCALIN] = check_and_cast<Ipv4NetfilterHook *>(getModuleByPath("^.inputHook"));
        hook[Ipv4Hook::NetfilterType::FORWARD] = check_and_cast<Ipv4NetfilterHook *>(getModuleByPath("^.forwardHook"));
        hook[Ipv4Hook::NetfilterType::POSTROUTING] = check_and_cast<Ipv4NetfilterHook *>(getModuleByPath("^.postroutingHook"));
        hook[Ipv4Hook::NetfilterType::LOCALOUT] = check_and_cast<Ipv4NetfilterHook *>(getModuleByPath("^.outputHook"));
    }
}

void Ipv4HookManager::registerNetfilterHandler(Ipv4Hook::NetfilterType type, int priority, Ipv4Hook::NetfilterHandler *handler)
{
    Enter_Method(__FUNCTION__);

    ASSERT(type >= 0 && type < Ipv4Hook::NetfilterType::__NUM_HOOK_TYPES);
    hook[type]->registerNetfilterHandler(priority, handler);
}

void Ipv4HookManager::unregisterNetfilterHandler(Ipv4Hook::NetfilterType type, int priority, Ipv4Hook::NetfilterHandler *handler)
{
    Enter_Method(__FUNCTION__);

    ASSERT(type >= 0 && type < Ipv4Hook::NetfilterType::__NUM_HOOK_TYPES);
    hook[type]->unregisterNetfilterHandler(priority, handler);
}

void Ipv4HookManager::reinjectDatagram(Packet *datagram, Ipv4Hook::NetfilterResult action)
{
    Enter_Method(__FUNCTION__);

    auto tag = datagram->getTag<Ipv4QueuedDatagramTag>();
    for (int hookPos = Ipv4Hook::NetfilterType::__FIRST_HOOK; hookPos < Ipv4Hook::NetfilterType::__NUM_HOOK_TYPES; ++hookPos) {
        if (hook[hookPos]->getId() == tag->getHookId()) {
            hook[hookPos]->reinjectQueuedDatagram(datagram, action);
            return;
        }
    }
    throw cRuntimeError("Hook not found for ID=%d", tag->getHookId());
}

} // namespace inet
