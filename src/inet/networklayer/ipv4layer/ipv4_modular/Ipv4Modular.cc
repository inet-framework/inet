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

void Ipv4Modular::chkHookManager()
{
    if (!hookManager)
        throw cRuntimeError("No hook manager, hooks not supported");
}

void Ipv4Modular::reinjectDatagram(Packet *datagram, Ipv4Hook::NetfilterResult action)
{
    Enter_Method(__FUNCTION__);

    chkHookManager();
    hookManager->reinjectDatagram(datagram, action);
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
