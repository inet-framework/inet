//
// Copyright (C) 2022 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/networklayer/ipv4layer/ipv4_modular/Ipv4NetfilterHook.h"

#include "inet/networklayer/contract/netfilter/NetfilterQueuedDatagramTag_m.h"

namespace inet {

Define_Module(Ipv4NetfilterHook);

void Ipv4NetfilterHook::registerNetfilterHandler(int priority, NetfilterHook::NetfilterHandler *handler)
{
    Enter_Method(__FUNCTION__);

    auto it = handlers.begin();

    for ( ; it != handlers.end(); ++it) {
        if (priority < it->priority) {
            break;
        }
        else if (it->priority == priority && it->handler == handler)
            throw cRuntimeError("handler already registered");
    }
    Item item(priority, handler);
    handlers.insert(it, item);
}

void Ipv4NetfilterHook::unregisterNetfilterHandler(int priority, NetfilterHook::NetfilterHandler *handler)
{
    Enter_Method(__FUNCTION__);

    for (auto it = handlers.begin(); it != handlers.end(); ++it) {
        if (priority < it->priority)
            break;
        else if (it->priority == priority && it->handler == handler) {
            handlers.erase(it);
            return;
        }
    }
    throw cRuntimeError("handler not found");
}

void Ipv4NetfilterHook::pushPacket(Packet *packet, cGate *gate)
{
    Enter_Method("pushPacket");

    take(packet);
    handlePacketProcessed(packet);

    if (iterateHandlers(packet) == NetfilterHook::NetfilterResult::ACCEPT)
        pushOrSendPacket(packet, outputGate, consumer);
}

Ipv4NetfilterHook::Items::iterator Ipv4NetfilterHook::findHandler(int priority, const NetfilterHook::NetfilterHandler *handler)
{
    auto it = handlers.begin();
    for ( ; it != handlers.end(); ++it) {
        if (it->priority == priority && it->handler == handler)
            return it;
        if (priority < it->priority)
            break;
    }
    return handlers.end();
}

NetfilterHook::NetfilterResult Ipv4NetfilterHook::iterateHandlers(Packet *packet, Items::iterator it)
{
    for ( ; it != handlers.end(); ++it) {
        NetfilterHook::NetfilterResult r = (*(it->handler))(packet);
        switch (r) {
            case NetfilterHook::NetfilterResult::ACCEPT:
                break; // continue iteration

            case NetfilterHook::NetfilterResult::DROP:
                //TODO emit signal
                delete packet;
                return r;

            case NetfilterHook::NetfilterResult::QUEUE: {
                if (packet->getOwner() != this)
                    throw cRuntimeError("Model error: netfilter handler changed the owner of queued datagram '%s'", packet->getFullName());
                auto tag = packet->addTag<NetfilterHook::NetfilterQueuedDatagramTag>();
                tag->setHookId(getId());
                tag->setPriority(it->priority);
                tag->setHandler(it->handler);
                return r;
            }

            case NetfilterHook::NetfilterResult::STOLEN:
                return r;

            default:
                throw cRuntimeError("Unknown IHook::Result value: %d", (int)r);
        }
    }
    return NetfilterHook::NetfilterResult::ACCEPT;
}

void Ipv4NetfilterHook::reinjectDatagram(Packet *packet, NetfilterHook::NetfilterResult action)
{
    Enter_Method("reinjectDatagram()");
    take(packet);
    auto tag = packet->getTag<NetfilterHook::NetfilterQueuedDatagramTag>();
    if (tag->getHookId() != getId())
        throw cRuntimeError("model error: Packet queued by another netfilter hook.");
    auto priority = tag->getPriority();
    auto handler = tag->getHandler();
    auto it = findHandler(priority, handler);
    if (it == handlers.end())
        throw cRuntimeError("hook not found for reinjected packet");
    switch (action) {
        case NetfilterHook::NetfilterResult::DROP:
            // TODO emit signal
            delete packet;
            break;

        case NetfilterHook::NetfilterResult::ACCEPT:
            ++it;
            // continue
        case NetfilterHook::NetfilterResult::REPEAT:
            if (iterateHandlers(packet, it) == NetfilterHook::NetfilterResult::ACCEPT)
                pushOrSendPacket(packet, outputGate, consumer);
            break;

        case NetfilterHook::NetfilterResult::STOP:
            pushOrSendPacket(packet, outputGate, consumer);
            break;

        default:
            throw cRuntimeError("Unaccepted NetfilterHook::NetfilterResult %i", (int)action);
    }
}

} // namespace inet
