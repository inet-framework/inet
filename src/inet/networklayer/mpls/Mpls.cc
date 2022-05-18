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
#include "inet/common/packet/Packet.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/ldp/Ldp.h"
#include "inet/networklayer/mpls/IIngressClassifier.h"
#include "inet/networklayer/rsvpte/Utils.h"
#include "inet/transportlayer/tcp_common/TcpHeader.h"

namespace inet {

#define ICMP_TRAFFIC    6

Define_Module(Mpls);

void Mpls::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        // interfaceTable must be initialized

        lt.reference(this, "libTableModule", true);
        ift.reference(this, "interfaceTableModule", true);
        pct.reference(this, "classifierModule", true);
    }
    else if (stage == INITSTAGE_NETWORK_LAYER) {
        registerService(Protocol::mpls, gate("upperLayerIn"), gate("upperLayerOut"));
        registerProtocol(Protocol::mpls, gate("lowerLayerOut"), gate("lowerLayerIn"));
    }
}

void Mpls::handleMessage(cMessage *msg)
{
    Packet *pk = check_and_cast<Packet *>(msg);
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
    using namespace tcp;

    const Protocol *protocol = msg->getTag<PacketProtocolTag>()->getProtocol();
    if (protocol != &Protocol::ipv4) {
        // only the Ipv4 protocol supported yet
        sendToL2(msg);
        return;
    }

    const auto& ipHeader = msg->peekAtFront<Ipv4Header>();

    // TODO temporary solution, until TcpSocket and Ipv4 are extended to support nam tracing
    if (ipHeader->getProtocolId() == IP_PROT_TCP) {
        const auto& seg = msg->peekDataAt<TcpHeader>(ipHeader->getChunkLength());
        if (seg->getDestPort() == LDP_PORT || seg->getSrcPort() == LDP_PORT) {
            ASSERT(!msg->hasPar("color"));
            msg->addPar("color") = LDP_TRAFFIC;
        }
    }
    else if (ipHeader->getProtocolId() == IP_PROT_ICMP) {
//        ASSERT(!msg->hasPar("color")); TODO this did not hold sometimes...
        if (!msg->hasPar("color"))
            msg->addPar("color") = ICMP_TRAFFIC;
    }
    // TODO end of temporary area

    labelAndForwardIpv4Datagram(msg);
}

bool Mpls::tryLabelAndForwardIpv4Datagram(Packet *packet)
{
    const auto& ipv4Header = packet->peekAtFront<Ipv4Header>();
    (void)ipv4Header; // unused variable
    LabelOpVector outLabel;
    std::string outInterface; // FIXME set based on interfaceID
    int color;

    if (!pct->lookupLabel(packet, outLabel, outInterface, color)) {
        EV_WARN << "no mapping exists for this packet" << endl;
        return false;
    }
    int outInterfaceId = CHK(ift->findInterfaceByName(outInterface.c_str()))->getInterfaceId();

    ASSERT(outLabel.size() > 0);

    doStackOps(packet, outLabel);

    EV_INFO << "forwarding packet to " << outInterface << endl;

    packet->addPar("color") = color;

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

void Mpls::popLabel(Packet *packet)
{
    ASSERT(packet->getTag<PacketProtocolTag>()->getProtocol()->getId() == Protocol::mpls.getId());
    auto oldMplsHeader = packet->popAtFront<MplsHeader>();
    if (oldMplsHeader->getS()) {
        packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::ipv4);
    }
}

void Mpls::doStackOps(Packet *packet, const LabelOpVector& outLabel)
{
    unsigned int n = outLabel.size();

    EV_INFO << "doStackOps: " << outLabel << endl;

    for (unsigned int i = 0; i < n; i++) {
        switch (outLabel[i].optcode) {
            case PUSH_OPER: {
                auto mplsHeader = makeShared<MplsHeader>();
                mplsHeader->setLabel(outLabel[i].label);
                pushLabel(packet, mplsHeader);
                break;
            }
            case SWAP_OPER: {
                auto mplsHeader = makeShared<MplsHeader>();
                mplsHeader->setLabel(outLabel[i].label);
                swapLabel(packet, mplsHeader);
                break;
            }
            case POP_OPER:
                popLabel(packet);
                break;

            default:
                throw cRuntimeError("Unknown MPLS OptCode %d", outLabel[i].optcode);
                break;
        }
    }
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
        throw cRuntimeError("Unknown message received");
        // FIXME remove throw below
        sendToL3(packet);
    }
}

void Mpls::processMplsPacketFromL2(Packet *packet)
{
    int incomingInterfaceId = packet->getTag<InterfaceInd>()->getInterfaceId();
    NetworkInterface *ie = ift->getInterfaceById(incomingInterfaceId);
    std::string incomingInterfaceName = ie->getInterfaceName();
    const auto& mplsHeader = packet->peekAtFront<MplsHeader>();

    EV_INFO << "Received " << packet << " from L2, label=" << mplsHeader->getLabel() << " inInterface=" << incomingInterfaceName << endl;

    if (mplsHeader->getLabel() == (uint32_t)-1) { // FIXME
        // This is a Ipv4 native packet (RSVP/TED traffic)
        // Decapsulate the message and pass up to L3
        EV_INFO << ": decapsulating and sending up\n";
        packet->popAtFront<MplsHeader>();
        packet->getTagForUpdate<PacketProtocolTag>()->setProtocol(&Protocol::ipv4);
        sendToL3(packet);
        return;
    }

    LabelOpVector outLabel;
    std::string outInterface;
    int color;

    bool found = lt->resolveLabel(incomingInterfaceName, mplsHeader->getLabel(), outLabel, outInterface, color);
    if (!found) {
        EV_INFO << "discarding packet, incoming label not resolved" << endl;

        delete packet;
        return;
    }

    NetworkInterface *outgoingInterface = CHK(ift->findInterfaceByName(outInterface.c_str()));

    doStackOps(packet, outLabel);

    if ((packet->getTag<PacketProtocolTag>()->getProtocol()->getId() == Protocol::mpls.getId())) {
        // forward labeled packet
        EV_INFO << "forwarding packet to " << outInterface << endl;

        if (packet->hasPar("color")) {
            packet->par("color") = color;
        }
        else {
            packet->addPar("color") = color;
        }

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
    send(msg, "lowerLayerOut");
}

void Mpls::sendToL3(Packet *msg)
{
    ASSERT(msg->findTag<InterfaceInd>());
    ASSERT(msg->findTag<DispatchProtocolReq>());
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

