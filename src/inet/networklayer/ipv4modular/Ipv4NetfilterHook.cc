//
// Copyright (C) 2022 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/networklayer/ipv4modular/Ipv4NetfilterHook.h"

#include "inet/networklayer/ipv4modular/Ipv4QueuedDatagramTag_m.h"

namespace inet {

Define_Module(Ipv4NetfilterHook);

void Ipv4NetfilterHook::registerNetfilterHandler(int priority, Ipv4Hook::NetfilterHandler *handler)
{
    auto it = handlers.begin();

    for ( ; it != handlers.end(); ++it) {
        if (priority > it->priority) {
            break;
        }
        else if (it->priority == priority && it->handler == handler)
            throw cRuntimeError("handler already registered");
    }
    Item item(priority, handler);
    handlers.insert(it, item);
}

void Ipv4NetfilterHook::unregisterNetfilterHandler(int priority, Ipv4Hook::NetfilterHandler *handler)
{
    for (auto it = handlers.begin(); it != handlers.end(); ++it) {
        if (priority > it->priority)
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

    if (iterateHandlers(packet) == Ipv4Hook::NetfilterResult::ACCEPT)
        pushOrSendPacket(packet, outputGate, consumer);
}

Ipv4NetfilterHook::Items::iterator Ipv4NetfilterHook::findHandler(int priority, const Ipv4Hook::NetfilterHandler *handler)
{
    auto it = handlers.begin();
    for ( ; it != handlers.end(); ++it) {
        if (it->priority == priority && it->handler == handler)
            return it;
        if (priority > it->priority)
            break;
    }
    return handlers.end();
}

Ipv4Hook::NetfilterResult Ipv4NetfilterHook::iterateHandlers(Packet *packet, Items::iterator it)
{
    for ( ; it != handlers.end(); ++it) {
        Ipv4Hook::NetfilterResult r = (*(it->handler))(packet);
        switch (r) {
            case Ipv4Hook::NetfilterResult::ACCEPT:
                break; // continue iteration

            case Ipv4Hook::NetfilterResult::DROP:
                //TODO emit signal
                delete packet;
                return r;

            case Ipv4Hook::NetfilterResult::QUEUE: {
                if (packet->getOwner() != this)
                    throw cRuntimeError("Model error: netfilter handler changed the owner of queued datagram '%s'", packet->getFullName());
                auto tag = packet->addTag<Ipv4QueuedDatagramTag>();
                tag->setHookId(getId());
                tag->setPriority(it->priority);
                tag->setHandler(it->handler);
                return r;
            }

            case Ipv4Hook::NetfilterResult::STOLEN:
                return r;

            default:
                throw cRuntimeError("Unknown IHook::Result value: %d", (int)r);
        }
    }
    return Ipv4Hook::NetfilterResult::ACCEPT;
}

void Ipv4NetfilterHook::reinjectQueuedDatagram(Packet *packet, Ipv4Hook::NetfilterResult action)
{
    Enter_Method("reinjectDatagram()");
    take(packet);
    auto tag = packet->getTag<Ipv4QueuedDatagramTag>();
    if (tag->getHookId() != getId())
        throw cRuntimeError("model error: Packet queued by another netfilter hook.");
    auto priority = tag->getPriority();
    auto handler = tag->getHandler();
    auto it = findHandler(priority, handler);
    if (it == handlers.end())
        throw cRuntimeError("hook not found for reinjected packet");
    switch (action) {
        case Ipv4Hook::NetfilterResult::DROP:
            delete packet;
            break;

        case Ipv4Hook::NetfilterResult::ACCEPT:
            ++it;
            // continue
        case Ipv4Hook::NetfilterResult::REPEAT:
            if (iterateHandlers(packet, it) == Ipv4Hook::NetfilterResult::ACCEPT)
                pushOrSendPacket(packet, outputGate, consumer);
            break;

        case Ipv4Hook::NetfilterResult::STOP:
            pushOrSendPacket(packet, outputGate, consumer);
            break;

        default:
            throw cRuntimeError("Unaccepted Ipv4Hook::NetfilterResult %i", (int)action);
    }
}

} // namespace inet
