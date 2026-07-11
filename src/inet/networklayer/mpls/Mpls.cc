//
// Copyright (C) 2005 Vojtech Janota
// Copyright (C) 2003 Xuan Thang Nguyen
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/networklayer/mpls/Mpls.h"

#include <string.h>

#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/Simsignals.h"
#include "inet/common/packet/Packet.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/L3Tools.h"
#include "inet/networklayer/mpls/IIngressClassifier.h"

namespace inet {

Define_Module(Mpls);

void Mpls::initialize(int stage)
{
    SimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        // interfaceTable must be initialized

        lt.reference(this, "libTableModule", true);
        ift.reference(this, "interfaceTableModule", true);
        pct.reference(this, "classifierModule", true);

        ttlModel = strcmp(par("ttlModel").stringValue(), "pipe") == 0 ? TTL_MODEL_PIPE : TTL_MODEL_UNIFORM;
        defaultTtl = par("defaultTtl");
        writeTcBackOnPop = par("writeTcBackOnPop");

        WATCH(numReceived);
        WATCH(numSent);
    }
    else if (stage == INITSTAGE_NETWORK_LAYER) {
        registerService(Protocol::mpls, gate("upperLayerIn"), gate("upperLayerOut"));
        registerProtocol(Protocol::mpls, gate("lowerLayerOut"), gate("lowerLayerIn"));
    }
}

void Mpls::handleMessage(cMessage *msg)
{
    Packet *pk = check_and_cast<Packet *>(msg);
    numReceived++;
    if (msg->getArrivalGate()->isName("lowerLayerIn")) {
        EV_INFO << "Processing message from L2: " << pk << endl;
        processPacketFromL2(pk);
    }
    else if (msg->getArrivalGate()->isName("upperLayerIn")) {
        EV_INFO << "Processing message from L3: " << pk << endl;
        processPacketFromL3(pk);
    }
    else {
        throw cRuntimeError("unexpected message: %s", msg->getName());
    }
}

void Mpls::processPacketFromL3(Packet *msg)
{
    const Protocol *protocol = msg->getTag<PacketProtocolTag>()->getProtocol();
    if (protocol != &Protocol::ipv4) {
        // only the Ipv4 protocol supported yet
        sendToL2(msg);
        return;
    }

    labelAndForwardIpv4Datagram(msg);
}

bool Mpls::tryLabelAndForwardIpv4Datagram(Packet *packet)
{
    LabelOpVector outLabel;
    int outInterfaceId;

    if (!pct->lookupLabel(packet, outLabel, outInterfaceId)) {
        EV_WARN << "no mapping exists for this packet" << endl;
        return false;
    }

    ASSERT(outLabel.size() > 0);

    if (!doStackOps(packet, outLabel, outInterfaceId))
        return true; // a freshly pushed label can never expire; kept for symmetry with processMplsPacketFromL2

    EV_INFO << "forwarding packet to " << ift->getInterfaceById(outInterfaceId)->getInterfaceName() << endl;

    packet->trim();
    packet->removeTagIfPresent<DispatchProtocolReq>();
    packet->addTagIfAbsent<InterfaceReq>()->setInterfaceId(outInterfaceId);
    sendToL2(packet);

    return true;
}

void Mpls::labelAndForwardIpv4Datagram(Packet *ipdatagram)
{
    if (tryLabelAndForwardIpv4Datagram(ipdatagram))
        return;

    // handling our outgoing Ipv4 traffic that didn't match any FEC/LSP
    // do not use labelAndForwardIPv4Datagram for packets arriving to ingress!

    EV_INFO << "FEC not resolved, doing regular L3 routing" << endl;

    sendToL2(ipdatagram);
}

void Mpls::pushLabel(Packet *packet, Ptr<MplsHeader>& newMplsHeader)
{
    packet->trimFront();
    newMplsHeader->setS(packet->getTag<PacketProtocolTag>()->getProtocol()->getId() != Protocol::mpls.getId());
    packet->insertAtFront(newMplsHeader);
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::mpls);
}

void Mpls::swapLabel(Packet *packet, Ptr<MplsHeader>& newMplsHeader)
{
    ASSERT(packet->getTag<PacketProtocolTag>()->getProtocol()->getId() == Protocol::mpls.getId());
    packet->trimFront();
    auto oldMplsHeader = packet->removeAtFront<MplsHeader>();
    newMplsHeader->setS(oldMplsHeader->getS());
    packet->insertAtFront(newMplsHeader);
}

void Mpls::popLabel(Packet *packet, const Protocol *payloadProtocol)
{
    ASSERT(packet->getTag<PacketProtocolTag>()->getProtocol()->getId() == Protocol::mpls.getId());
    auto oldMplsHeader = packet->popAtFront<MplsHeader>();
    if (oldMplsHeader->getS()) {
        packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(payloadProtocol);

        // RFC 3443 §3.2: in uniform mode the (possibly decremented) label TTL is
        // authoritative and is written back into the IP header; in pipe mode the
        // original IP TTL was never touched by the MPLS domain, so it is left alone.
        // This is Ipv4Header-specific; IPv6 hop-limit handling is Workstream F3 Phase
        // 2 -- guarded out here so a (currently nonexistent) IPv6 LIB entry doesn't
        // crash trying to treat a non-Ipv4Header packet as Ipv4Header.
        if (payloadProtocol == &Protocol::ipv4 && ttlModel == TTL_MODEL_UNIFORM) {
            auto ipv4Header = removeNetworkProtocolHeader<Ipv4Header>(packet);
            ipv4Header->setTimeToLive(oldMplsHeader->getTtl());
            // writeTcBackOnPop defaults to false: the 3-bit tc -> 6-bit DSCP mapping
            // is lossy, and RFC 5462 (short pipe) keeps the inner DSCP authoritative
            if (writeTcBackOnPop)
                ipv4Header->setDscp(oldMplsHeader->getTc() << 3);
            ipv4Header->updateChecksum();
            insertNetworkProtocolHeader(packet, Protocol::ipv4, ipv4Header);
        }
    }
}

uint8_t Mpls::computePushTtl(const Packet *packet) const
{
    if (packet->getTag<PacketProtocolTag>()->getProtocol()->getId() == Protocol::mpls.getId())
        // pushing an additional label onto an existing stack: copy the current outer label's TTL
        return packet->peekAtFront<MplsHeader>()->getTtl();

    const auto& ipv4Header = packet->peekAtFront<Ipv4Header>();
    return ttlModel == TTL_MODEL_UNIFORM ? (uint8_t)ipv4Header->getTimeToLive() : (uint8_t)defaultTtl;
}

uint8_t Mpls::computePushTc(const Packet *packet) const
{
    if (packet->getTag<PacketProtocolTag>()->getProtocol()->getId() == Protocol::mpls.getId())
        // pushing an additional label onto an existing stack: copy the current outer label's tc
        return packet->peekAtFront<MplsHeader>()->getTc();

    // E-LSP default mapping: the top 3 bits of the 6-bit DSCP become the MPLS Traffic Class
    const auto& ipv4Header = packet->peekAtFront<Ipv4Header>();
    return (uint8_t)(ipv4Header->getDscp() >> 3);
}

void Mpls::handleTtlExpiry(Packet *packet, int outInterfaceId)
{
    EV_WARN << "MPLS label TTL expired, discarding the label stack and handing the datagram up to L3 for ICMP Time Exceeded processing" << endl;

    // pop every remaining label (RFC 3443: do not forward the packet any further as MPLS)
    for (bool bottomOfStack = false; !bottomOfStack; ) {
        auto oldMplsHeader = packet->popAtFront<MplsHeader>();
        bottomOfStack = oldMplsHeader->getS();
    }
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::ipv4);

    // write the expired TTL back into the IP header; this happens in both TTL
    // models (RFC 3443: expiry handling is common to uniform and pipe modes)
    auto ipv4Header = removeNetworkProtocolHeader<Ipv4Header>(packet);
    ipv4Header->setTimeToLive(0);
    ipv4Header->updateChecksum();
    insertNetworkProtocolHeader(packet, Protocol::ipv4, ipv4Header);

    // hand the packet to L3 as if it just arrived from the network on the interface
    // this LSR would have forwarded it on; this LSR (and its peers) typically has no
    // IP route to the packet's ultimate destination (only the MPLS LIB knows the path),
    // so the outgoing interface resolved by the label lookup is supplied explicitly to
    // avoid a spurious "destination unreachable" -- Ipv4::fragmentAndSend's own
    // hop-count check then generates the real ICMP Time Exceeded
    packet->addTagIfAbsent<InterfaceReq>()->setInterfaceId(outInterfaceId);
    deliverToL3(packet);
}

void Mpls::deliverToL3(Packet *packet)
{
    packet->removeTagIfPresent<DispatchProtocolReq>();
    packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(&Protocol::ipv4);
    sendToL3(packet);
}

bool Mpls::doStackOps(Packet *packet, const LabelOpVector& outLabel, int outInterfaceId,
        const Protocol *payloadProtocol)
{
    unsigned int n = outLabel.size();

    EV_INFO << "doStackOps: " << outLabel << endl;

    for (unsigned int i = 0; i < n; i++) {
        switch (outLabel[i].optcode) {
            case PUSH_OPER: {
                auto mplsHeader = makeShared<MplsHeader>();
                mplsHeader->setLabel(outLabel[i].label);
                mplsHeader->setTtl(computePushTtl(packet));
                mplsHeader->setTc(computePushTc(packet));
                pushLabel(packet, mplsHeader);
                break;
            }
            case SWAP_OPER: {
                ASSERT(packet->getTag<PacketProtocolTag>()->getProtocol()->getId() == Protocol::mpls.getId());
                const auto& oldMplsHeader = packet->peekAtFront<MplsHeader>();
                uint8_t oldTtl = oldMplsHeader->getTtl();
                if (oldTtl <= 1) {
                    handleTtlExpiry(packet, outInterfaceId);
                    return false;
                }
                auto mplsHeader = makeShared<MplsHeader>();
                mplsHeader->setLabel(outLabel[i].label);
                mplsHeader->setTtl(oldTtl - 1);
                mplsHeader->setTc(oldMplsHeader->getTc());
                swapLabel(packet, mplsHeader);
                break;
            }
            case POP_OPER:
                popLabel(packet, payloadProtocol);
                break;

            default:
                throw cRuntimeError("Unknown MPLS OptCode %d", outLabel[i].optcode);
                break;
        }
    }
    return true;
}

void Mpls::processPacketFromL2(Packet *packet)
{
    int protocolId = packet->getTag<PacketProtocolTag>()->getProtocol()->getId();
    if (protocolId == Protocol::mpls.getId()) {
        processMplsPacketFromL2(packet);
    }
    else if (protocolId == Protocol::ipv4.getId()) {
        // Ipv4 datagram arrives at Ingress router. We'll try to classify it
        // and add an MPLS header

        if (!tryLabelAndForwardIpv4Datagram(packet)) {
            sendToL3(packet);
        }
    }
    else {
        // not an IPv4 or MPLS packet (e.g. IPv6): this model cannot label or
        // forward it, so drop it with a notification instead of aborting
        EV_WARN << "discarding packet from L2 with unsupported protocol "
                << packet->getTag<PacketProtocolTag>()->getProtocol()->getName() << endl;
        PacketDropDetails details;
        details.setReason(NO_PROTOCOL_FOUND);
        emit(packetDroppedSignal, packet, &details);
        delete packet;
    }
}

void Mpls::processMplsPacketFromL2(Packet *packet)
{
    int incomingInterfaceId = packet->getTag<InterfaceInd>()->getInterfaceId();
    const auto& mplsHeader = packet->peekAtFront<MplsHeader>();
    uint32_t label = mplsHeader->getLabel();

    EV_INFO << "Received " << packet << " from L2, label=" << label << " inInterface=" << ift->getInterfaceById(incomingInterfaceId)->getInterfaceName() << endl;

    if (label == IPV4_EXPLICIT_NULL_LABEL) {
        // RFC 3032: the explicit null label always means "pop and process the
        // datagram as ordinary IPv4", regardless of what the LIB says
        EV_INFO << "explicit null label, popping and delivering to L3" << endl;
        popLabel(packet);
        packet->trim();
        deliverToL3(packet);
        return;
    }
    if (label > IMPLICIT_NULL_LABEL && label <= RESERVED_LABEL_MAX) {
        // RFC 3032: labels 4-15 are reserved but currently unassigned
        EV_WARN << "discarding packet with unassigned reserved label " << label << endl;
        PacketDropDetails details;
        details.setReason(OTHER_PACKET_DROP);
        emit(packetDroppedSignal, packet, &details);
        delete packet;
        return;
    }

    LabelOpVector outLabel;
    int outInterfaceId;
    const Protocol *payloadProtocol;

    bool found = lt->resolveLabel(incomingInterfaceId, label, outLabel, outInterfaceId, payloadProtocol);
    if (!found) {
        EV_INFO << "discarding packet, incoming label not resolved" << endl;

        delete packet;
        return;
    }

    NetworkInterface *outgoingInterface = ift->getInterfaceById(outInterfaceId);

    if (!doStackOps(packet, outLabel, outInterfaceId, payloadProtocol))
        return; // TTL expired: the datagram was already popped and handed to L3

    if ((packet->getTag<PacketProtocolTag>()->getProtocol()->getId() == Protocol::mpls.getId())) {
        // forward labeled packet
        EV_INFO << "forwarding packet to " << outgoingInterface->getInterfaceName() << endl;

//        ASSERT(labelIf[outgoingPort]);
        packet->removeTagIfPresent<DispatchProtocolReq>();
        packet->addTagIfAbsent<InterfaceReq>()->setInterfaceId(outgoingInterface->getInterfaceId());
        packet->trim();
        sendToL2(packet);
    }
    else {
        // last label popped, decapsulate and send out Ipv4 datagram

        EV_INFO << "decapsulating Ipv4 datagram" << endl;
        ASSERT(packet->getTag<PacketProtocolTag>()->getProtocol()->getId() == Protocol::ipv4.getId());

        if (outgoingInterface) {
            packet->trim();
            packet->removeTagIfPresent<DispatchProtocolReq>();
            packet->addTagIfAbsent<InterfaceReq>()->setInterfaceId(outgoingInterface->getInterfaceId());
            sendToL2(packet);
        }
        else {
            sendToL3(packet);
        }
    }
}

void Mpls::sendToL2(Packet *msg)
{
    ASSERT(msg->findTag<InterfaceReq>());
    ASSERT(msg->findTag<PacketProtocolTag>());
    numSent++;
    send(msg, "lowerLayerOut");
}

void Mpls::sendToL3(Packet *msg)
{
    ASSERT(msg->findTag<InterfaceInd>());
    ASSERT(msg->findTag<DispatchProtocolReq>());
    numSent++;
    send(msg, "upperLayerOut");
}

void Mpls::handleRegisterInterface(const NetworkInterface& interface, cGate *out, cGate *in)
{
    if (!strcmp("lowerLayerIn", in->getBaseName()))
        registerInterface(interface, gate("upperLayerIn"), gate("upperLayerOut"));
}

void Mpls::handleRegisterService(const Protocol& protocol, cGate *g, ServicePrimitive servicePrimitive)
{
    Enter_Method("handleRegisterService");
    if (g->isName("lowerLayerOut"))
        registerService(protocol, gate("upperLayerIn"), servicePrimitive);
    else if (g->isName("lowerLayerIn"))
        ;
    else if (g->isName("upperLayerOut"))
        registerService(protocol, gate("lowerLayerIn"), servicePrimitive);
    else if (g->isName("upperLayerIn"))
        ;
    else
        throw cRuntimeError("Unknown gate: %s", g->getName());
}

void Mpls::handleRegisterProtocol(const Protocol& protocol, cGate *g, ServicePrimitive servicePrimitive)
{
    Enter_Method("handleRegisterProtocol");
    if (g->isName("lowerLayerIn"))
        registerProtocol(protocol, gate("upperLayerOut"), servicePrimitive);
    else if (g->isName("lowerLayerOut"))
        ;
    else if (g->isName("upperLayerOut"))
        registerProtocol(protocol, gate("lowerLayerIn"), servicePrimitive);
    else if (g->isName("upperLayerIn"))
        ; // void
    else
        throw cRuntimeError("Unknown gate: %s", g->getName());
}

} // namespace inet

