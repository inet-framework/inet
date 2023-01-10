//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/ProtocolUtils.h"

namespace inet {

void prependEncapsulationProtocolReq(Packet *packet, const Protocol *protocol)
{
    auto encapsulationProtocolReq = packet->addTagIfAbsent<EncapsulationProtocolReq>();
    encapsulationProtocolReq->insertProtocol(0, protocol);
}

void appendEncapsulationProtocolReq(Packet *packet, const Protocol *protocol)
{
    auto encapsulationProtocolReq = packet->addTagIfAbsent<EncapsulationProtocolReq>();
    encapsulationProtocolReq->appendProtocol(protocol);
}

const Protocol *peekEncapsulationProtocolReq(Packet *packet)
{
    if (auto encapsulationProtocolReq = packet->findTagForUpdate<EncapsulationProtocolReq>())
        return encapsulationProtocolReq->getProtocol(0);
    else
        return nullptr;
}

const Protocol *popEncapsulationProtocolReq(Packet *packet)
{
    if (auto encapsulationProtocolReq = packet->findTagForUpdate<EncapsulationProtocolReq>()) {
        auto encapsulationProtocol = encapsulationProtocolReq->getProtocol(0);
        encapsulationProtocolReq->eraseProtocol(0);
        if (encapsulationProtocolReq->getProtocolArraySize() == 0)
            packet->removeTag<EncapsulationProtocolReq>();
        return encapsulationProtocol;
    }
    else
        return nullptr;
}

INET_API bool hasEncapsulationProtocolReq(Packet *packet, const Protocol *protocol)
{
    if (auto encapsulationProtocolReq = packet->addTagIfAbsent<EncapsulationProtocolReq>()) {
        for (int i = 0; i < encapsulationProtocolReq->getProtocolArraySize(); i++)
            if (encapsulationProtocolReq->getProtocol(i) == protocol)
                return true;
    }
    return false;
}

void removeEncapsulationProtocolReq(Packet *packet, const Protocol *protocol)
{
    if (auto encapsulationProtocolReq = packet->addTagIfAbsent<EncapsulationProtocolReq>()) {
        for (int i = 0; i < encapsulationProtocolReq->getProtocolArraySize(); i++) {
            if (encapsulationProtocolReq->getProtocol(i) == protocol) {
                encapsulationProtocolReq->eraseProtocol(i);
                break;
            }
        }
    }
}

void ensureEncapsulationProtocolReq(Packet *packet, const Protocol *protocol, bool present, bool prepend)
{
    if (present) {
        if (!hasEncapsulationProtocolReq(packet, protocol)) {
            if (prepend)
                prependEncapsulationProtocolReq(packet, protocol);
            else
                appendEncapsulationProtocolReq(packet, protocol);
        }
    }
    else {
        if (hasEncapsulationProtocolReq(packet, protocol))
            removeEncapsulationProtocolReq(packet, protocol);
    }
}

void prependEncapsulationProtocolInd(Packet *packet, const Protocol *protocol)
{
    auto encapsulationProtocolInd = packet->addTagIfAbsent<EncapsulationProtocolInd>();
    encapsulationProtocolInd->insertProtocol(0, protocol);
}

void appendEncapsulationProtocolInd(Packet *packet, const Protocol *protocol)
{
    auto encapsulationProtocolInd = packet->addTagIfAbsent<EncapsulationProtocolInd>();
    encapsulationProtocolInd->appendProtocol(protocol);
}

const Protocol *popEncapsulationProtocolInd(Packet *packet)
{
    if (auto encapsulationProtocolInd = packet->findTagForUpdate<EncapsulationProtocolInd>()) {
        auto encapsulationProtocol = encapsulationProtocolInd->getProtocol(0);
        encapsulationProtocolInd->eraseProtocol(0);
        if (encapsulationProtocolInd->getProtocolArraySize() == 0)
            packet->removeTag<EncapsulationProtocolInd>();
        return encapsulationProtocol;
    }
    else
        return nullptr;
}

void setDispatchProtocol(Packet *packet, const Protocol *defaultProtocol)
{
    if (auto protocol = peekEncapsulationProtocolReq(packet))
        packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(protocol);
    else if (defaultProtocol != nullptr)
        packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(defaultProtocol);
    else
        packet->removeTagIfPresent<DispatchProtocolReq>();
}

void removeDispatchProtocol(Packet *packet, const Protocol *expectedProtocol)
{
    if (auto dispatchProtocolReq = packet->findTag<DispatchProtocolReq>()) {
        ASSERT(dispatchProtocolReq->getProtocol() == expectedProtocol);
        packet->removeTag<DispatchProtocolReq>();
    }
    if (auto encapsulationProtocolReq = packet->findTag<EncapsulationProtocolReq>()) {
        ASSERT(encapsulationProtocolReq->getProtocolArraySize() > 0 && encapsulationProtocolReq->getProtocol(0) == expectedProtocol);
        popEncapsulationProtocolReq(packet);
    }
}

} // namespace inet

