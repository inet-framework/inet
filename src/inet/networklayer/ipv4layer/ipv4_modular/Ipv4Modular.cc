//
// Copyright (C) 2022 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/networklayer/ipv4layer/ipv4_modular/Ipv4Modular.h"

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/networklayer/contract/netfilter/NetfilterHookDefs_m.h"

namespace inet {

Define_Module(Ipv4Modular);

void Ipv4Modular::initialize(int stage)
{
    cModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        hookManager = check_and_cast_nullable<INetfilterHookManager *>(findModuleByPath(".hookManager"));
    }
    else if (stage == INITSTAGE_NETWORK_LAYER) {
        registerService(Protocol::ipv4, gate("transportIn"), gate("transportOut"));
        registerProtocol(Protocol::ipv4, gate("queueOut"), gate("queueIn"));
    }
}

void Ipv4Modular::chkHookManager()
{
    if (!hookManager)
        throw cRuntimeError("No hook manager, hooks not supported");
}

void Ipv4Modular::reinjectDatagram(Packet *datagram, NetfilterHook::NetfilterResult action)
{
    Enter_Method(__FUNCTION__);

    chkHookManager();
    hookManager->reinjectDatagram(datagram, action);
}

void Ipv4Modular::registerNetfilterHandler(NetfilterHook::NetfilterType type, int priority, NetfilterHook::NetfilterHandler *handler)
{
    Enter_Method(__FUNCTION__);

    chkHookManager();
    hookManager->registerNetfilterHandler(type, priority, handler);
}

void Ipv4Modular::unregisterNetfilterHandler(NetfilterHook::NetfilterType type, int priority, NetfilterHook::NetfilterHandler *handler)
{
    Enter_Method(__FUNCTION__);

    chkHookManager();
    hookManager->unregisterNetfilterHandler(type, priority, handler);
}

} // namespace inet
