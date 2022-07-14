//
// Copyright (C) 2022 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/networklayer/ipv4modular/Ipv4Modular.h"

#include "inet/common/INETDefs.h"
#include "inet/common/IProtocolRegistrationListener.h"

namespace inet {

Define_Module(Ipv4Modular);

void Ipv4Modular::initialize(int stage)
{
    cModule::initialize(stage);

    if (stage == INITSTAGE_NETWORK_LAYER) {
        registerService(Protocol::ipv4, gate("transportIn"), gate("transportOut"));
        registerProtocol(Protocol::ipv4, gate("queueOut"), gate("queueIn"));
    }
}

void Ipv4Modular::registerHook(int priority, IHook *hook)
{
    throw cRuntimeError("Hook management not implemented");
}

void Ipv4Modular::unregisterHook(IHook *hook)
{
    throw cRuntimeError("Hook management not implemented");
}

void Ipv4Modular::dropQueuedDatagram(const Packet *daragram)
{
    throw cRuntimeError("Hook management not implemented");
}

void Ipv4Modular::reinjectQueuedDatagram(const Packet *datagram)
{
    throw cRuntimeError("Hook management not implemented");
}

} // namespace inet
