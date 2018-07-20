//
// (C) 2005 Vojtech Janota
// (C) 2003 Xuan Thang Nguyen
//
// This library is free software, you can redistribute it
// and/or modify
// it under  the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//

#include <string.h>

#include "inet/common/INETDefs.h"

#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/Packet.h"

#include "inet/networklayer/mpls/Mpls.h"

#include "inet/common/ModuleAccess.h"
#include "inet/networklayer/mpls/IIngressClassifier.h"
#include "inet/networklayer/rsvpte/Utils.h"

// FIXME temporary fix
#include "inet/networklayer/ldp/Ldp.h"
#include "inet/transportlayer/tcp_common/TcpHeader.h"
#include "inet/linklayer/common/InterfaceTag_m.h"

namespace inet {

#define ICMP_TRAFFIC    6

Define_Module(Mpls);

void Mpls::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        // interfaceTable must be initialized

        lt = getModuleFromPar<LibTable>(par("libTableModule"), this);
        ift = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        pct = getModuleFromPar<IIngressClassifier>(par("classifierModule"), this);
    }
    else if (stage == INITSTAGE_NETWORK_LAYER) {
        registerService(Protocol::mpls, gate("netwIn"), gate("ifIn"));
        registerProtocol(Protocol::mpls, gate("ifOut"), gate("netwOut"));
    }
}

void Mpls::handleMessage(cMessage *msg)
{
    Packet *pk = check_and_cast<Packet *>(msg);
    if (msg->getArrivalGate()->isName("ifIn")) {
        EV_INFO << "Processing message from L2: " << pk << endl;
        processPacketFromL2(pk);
    }
    else if (msg->getArrivalGate()->isName("netwIn")) {
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

    // XXX temporary solution, until TcpSocket and Ipv4 are extended to support nam tracing
    if (ipHeader->getProtocolId() == IP_PROT_TCP) {
        const auto& seg = msg->peekDataAt<TcpHeader>(ipHeader->getChunkLength());
        if (seg->getDestPort() == LDP_PORT || seg->getSrcPort() == LDP_PORT) {
            ASSERT(!msg->hasPar("color"));
            msg->addPar("color") = LDP_TRAFFIC;
        }
    }
    else if (ipHeader->getProtocolId() == IP_PROT_ICMP) {
        // ASSERT(!msg->hasPar("color")); XXX this did not hold sometimes...
        if (!msg->hasPar("color"))
            msg->addPar("color") = ICMP_TRAFFIC;
    }
    // XXX end of temporary area

    labelAndForwardIpv4Datagram(msg);
}

bool Mpls::tryLabelAndForwardIpv4Datagram(Packet *packet)
{
    const auto& ipv4Header = packet->peekAtFront<Ipv4Header>();
    (void)ipv4Header;       // unused variable
    LabelOpVector outLabel;
    std::string outInterface;   //FIXME set based on interfaceID
    int color;

    if (!pct->lookupLabel(packet, outLabel, outInterface, color)) {
        EV_WARN << "no mapping exists for this packet" << endl;
        return false;
    }
    int outInterfaceId = CHK(ift->getInterfaceByName(outInterface.c_str()))->getInterfaceId();

    ASSERT(outLabel.size() > 0);

    const auto& mplsHeader = makeShared<MplsHeader>();
    doStackOps(mplsHeader.get(), outLabel);

    EV_INFO << "forwarding packet to " << outInterface << endl;

    packet->addPar("color") = color;

    if (!mplsHeader->hasLabel()) {
        // yes, this may happen - if we'are both ingress and egress
        packet->trim();
        packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::ipv4);
        delete packet->removeTagIfPresent<DispatchProtocolReq>();
        packet->addTagIfAbsent<InterfaceReq>()->setInterfaceId(outInterfaceId);
        sendToL2(packet);
    }
    else {
        packet->trim();
        packet->insertAtFront(mplsHeader);
        packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::mpls);
        delete packet->removeTagIfPresent<DispatchProtocolReq>();
        packet->addTagIfAbsent<InterfaceReq>()->setInterfaceId(outInterfaceId);
        sendToL2(packet);
    }

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

void Mpls::doStackOps(MplsHeader *mplsHeader, const LabelOpVector& outLabel)
{
    unsigned int n = outLabel.size();

    EV_INFO << "doStackOps: " << outLabel << endl;

    for (unsigned int i = 0; i < n; i++) {
        switch (outLabel[i].optcode) {
            case PUSH_OPER: {
                MplsLabel label;
                label.setLabel(outLabel[i].label);
                mplsHeader->pushLabel(label);
                break;
            }
            case SWAP_OPER: {
                ASSERT(mplsHeader->hasLabel());
                MplsLabel label = mplsHeader->getTopLabel();
                label.setLabel(outLabel[i].label);
                mplsHeader->swapLabel(label);
                break;
            }
            case POP_OPER:
                ASSERT(mplsHeader->hasLabel());
                mplsHeader->popLabel();
                break;

            default:
                throw cRuntimeError("Unknown MPLS OptCode %d", outLabel[i].optcode);
                break;
        }
    }
}

void Mpls::processPacketFromL2(Packet *packet)
{
    const Protocol *protocol = packet->getTag<PacketProtocolTag>()->getProtocol();
    if (protocol == &Protocol::mpls) {
        processMplsPacketFromL2(packet);
    }
    else if (protocol == &Protocol::ipv4) {
        // Ipv4 datagram arrives at Ingress router. We'll try to classify it
        // and add an MPLS header

        if (!tryLabelAndForwardIpv4Datagram(packet)) {
            sendToL3(packet);
        }
    }
    else {
        throw cRuntimeError("Unknown message received");
        //FIXME remove throw below
        sendToL3(packet);
    }
}

void Mpls::processMplsPacketFromL2(Packet *packet)
{
    int incomingInterfaceId = packet->getTag<InterfaceInd>()->getInterfaceId();
    InterfaceEntry *ie = ift->getInterfaceById(incomingInterfaceId);
    std::string incomingInterfaceName = ie->getInterfaceName();
    const auto& mplsHeader = dynamicPtrCast<MplsHeader>(packet->popAtFront<MplsHeader>()->dupShared());
    ASSERT(mplsHeader->hasLabel());
    MplsLabel oldLabel = mplsHeader->getTopLabel();

    EV_INFO << "Received " << packet << " from L2, label=" << oldLabel.getLabel() << " inInterface=" << incomingInterfaceName << endl;

    if (oldLabel.getLabel() == -1) {
        // This is a Ipv4 native packet (RSVP/TED traffic)
        // Decapsulate the message and pass up to L3
        EV_INFO << ": decapsulating and sending up\n";
        sendToL3(packet);
        return;
    }

    LabelOpVector outLabel;
    std::string outInterface;
    int color;

    bool found = lt->resolveLabel(incomingInterfaceName, oldLabel.getLabel(), outLabel, outInterface, color);
    if (!found) {
        EV_INFO << "discarding packet, incoming label not resolved" << endl;

        delete packet;
        return;
    }

    InterfaceEntry *outgoingInterface = ift->getInterfaceByName(outInterface.c_str());

    doStackOps(mplsHeader.get(), outLabel);

    if (mplsHeader->hasLabel()) {
        // forward labeled packet
        packet->trim();
        packet->insertAtFront(mplsHeader);

        EV_INFO << "forwarding packet to " << outInterface << endl;

        if (packet->hasPar("color")) {
            packet->par("color") = color;
        }
        else {
            packet->addPar("color") = color;
        }

        //ASSERT(labelIf[outgoingPort]);
        delete packet->removeTagIfPresent<DispatchProtocolReq>();
        packet->addTagIfAbsent<InterfaceReq>()->setInterfaceId(outgoingInterface->getInterfaceId());
        packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::mpls);
        sendToL2(packet);
    }
    else {
        // last label popped, decapsulate and send out Ipv4 datagram

        EV_INFO << "decapsulating Ipv4 datagram" << endl;

        if (outgoingInterface) {
            packet->trim();
            delete packet->removeTagIfPresent<DispatchProtocolReq>();
            packet->addTagIfAbsent<InterfaceReq>()->setInterfaceId(outgoingInterface->getInterfaceId());
            packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::ipv4); // TODO: this ipv4 protocol is a lie somewhat, because this is the mpls protocol
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
    send(msg, "ifOut");
}

void Mpls::sendToL3(Packet *msg)
{
    ASSERT(msg->findTag<InterfaceInd>());
    ASSERT(msg->findTag<DispatchProtocolReq>());
    send(msg, "netwOut");
}

void Mpls::handleRegisterInterface(const InterfaceEntry &interface, cGate *out, cGate *in)
{
    if (!strcmp("ifIn", in->getBaseName()))
        registerInterface(interface, gate("netwIn"), gate("netwOut"));
}

void Mpls::handleRegisterService(const Protocol& protocol, cGate *out, ServicePrimitive servicePrimitive)
{
    Enter_Method("handleRegisterService");
    if (!strcmp("ifOut", out->getName()))
        registerService(protocol, gate("netwIn"), servicePrimitive);
    else if (!strcmp("netwOut", out->getName()))
        registerService(protocol, gate("ifIn"), servicePrimitive);
    else
        throw cRuntimeError("Unknown gate: %s", out->getName());
}

void Mpls::handleRegisterProtocol(const Protocol& protocol, cGate *in, ServicePrimitive servicePrimitive)
{
    Enter_Method("handleRegisterProtocol");
    if (!strcmp("ifIn", in->getName()))
        registerProtocol(protocol, gate("netwOut"), servicePrimitive);
    else if (!strcmp("netwIn", in->getName()))
        registerProtocol(protocol, gate("ifOut"), servicePrimitive);
    else
        throw cRuntimeError("Unknown gate: %s", in->getName());
}

} // namespace inet

