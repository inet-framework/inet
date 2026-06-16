//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/networklayer/icmpv6/Mldv1.h"

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/Protocol.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/checksum/ChecksumMode_m.h"
#include "inet/common/packet/Message.h"
#include "inet/common/packet/Packet.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/HopLimitTag_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/icmpv6/MldMessage_m.h"
#include "inet/networklayer/ipv6/Ipv6RoutingTable.h"

namespace inet {

Define_Module(Mldv1);

void Mldv1::initialize(int stage)
{
    OperationalBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        enabled = par("enabled");
        const char *checksumModeString = par("checksumMode");
        checksumMode = parseChecksumMode(checksumModeString, false);
    }
    else if (stage == INITSTAGE_NETWORK_LAYER) {
        ift.reference(this, "interfaceTableModule", true);
        rt.reference(this, "routingTableModule", true);

        // Register Protocol::mld so the lp dispatcher delivers MLD ICMPv6 messages to us
        registerProtocol(Protocol::mld, gate("ipOut"), gate("ipIn"));
    }
}

void Mldv1::handleMessageWhenUp(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        // No timers in this phase (state machines deferred to Phases 3/4)
        throw cRuntimeError("Unexpected self-message: %s", msg->getName());
    }
    else if (auto packet = dynamic_cast<Packet *>(msg)) {
        if (packet->getTag<PacketProtocolTag>()->getProtocol() == &Protocol::mld) {
            processMldMessage(packet);
        }
        else
            throw cRuntimeError("Unknown message type received.");
    }
    else if (auto indication = dynamic_cast<Indication *>(msg)) {
        // The IPv6 layer reports an ICMPv6 error for an MLD message we sent;
        // discard it (no recovery logic in this skeleton phase).
        EV_WARN << "Received an error indication (" << indication->getName()
                << ") for an MLD message; ignoring it" << endl;
        delete indication;
    }
    else
        throw cRuntimeError("Unknown message '%s' (%s) received", msg->getName(), msg->getClassName());
}

void Mldv1::processMldMessage(Packet *packet)
{
    const auto& mldMsg = packet->peekAtFront<MldMessage>();
    EV_INFO << "Received MLD message, type=" << (int)mldMsg->getType() << endl;
    // Phase 3/4: dispatch to host/router state machines
    delete packet;
}

void Mldv1::sendToIPv6(Packet *msg, NetworkInterface *ie, const Ipv6Address& dest)
{
    ASSERT(ie->isMulticast());
    msg->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::mld);
    msg->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(&Protocol::ipv6);
    msg->addTagIfAbsent<InterfaceReq>()->setInterfaceId(ie->getInterfaceId());
    msg->addTagIfAbsent<L3AddressReq>()->setDestAddress(dest);
    msg->addTagIfAbsent<HopLimitReq>()->setHopLimit(1);   // RFC 2710 §3: hop limit = 1
    send(msg, "ipOut");
}

} // namespace inet
