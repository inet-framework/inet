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

#include "inet/networklayer/mpls/MPLS.h"
#include "inet/networklayer/rsvp_te/Utils.h"

#include "inet/networklayer/mpls/IClassifier.h"
#include "inet/common/ModuleAccess.h"

// FIXME temporary fix
#include "inet/networklayer/ldp/LDP.h"
#include "inet/transportlayer/tcp_common/TCPSegment.h"

namespace inet {

#define ICMP_TRAFFIC    6

Define_Module(MPLS);

void MPLS::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        // interfaceTable must be initialized

        lt = getModuleFromPar<LIBTable>(par("libTableModule"), this);
        ift = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);

        pct = check_and_cast<IClassifier *>(getParentModule()->getSubmodule(par("classifier")));
    }
}

void MPLS::handleMessage(cMessage *msg)
{
    if (!strcmp(msg->getArrivalGate()->getName(), "ifIn")) {
        EV_INFO << "Processing message from L2: " << msg << endl;
        processPacketFromL2(msg);
    }
    else if (!strcmp(msg->getArrivalGate()->getName(), "netwIn")) {
        EV_INFO << "Processing message from L3: " << msg << endl;
        processPacketFromL3(msg);
    }
    else {
        throw cRuntimeError("unexpected message: %s", msg->getName());
    }
}

void MPLS::sendToL2(cMessage *msg, int gateIndex)
{
    send(msg, "ifOut", gateIndex);
}

void MPLS::processPacketFromL3(cMessage *msg)
{
    using namespace tcp;

    IPv4Datagram *ipdatagram = check_and_cast<IPv4Datagram *>(msg);
    //int gateIndex = msg->getArrivalGate()->getIndex();

    // XXX temporary solution, until TCPSocket and IPv4 are extended to support nam tracing
    if (ipdatagram->getTransportProtocol() == IP_PROT_TCP) {
        TCPSegment *seg = check_and_cast<TCPSegment *>(ipdatagram->getEncapsulatedPacket());
        if (seg->getDestPort() == LDP_PORT || seg->getSrcPort() == LDP_PORT) {
            ASSERT(!ipdatagram->hasPar("color"));
            ipdatagram->addPar("color") = LDP_TRAFFIC;
        }
    }
    else if (ipdatagram->getTransportProtocol() == IP_PROT_ICMP) {
        // ASSERT(!ipdatagram->hasPar("color")); XXX this did not hold sometimes...
        if (!ipdatagram->hasPar("color"))
            ipdatagram->addPar("color") = ICMP_TRAFFIC;
    }
    // XXX end of temporary area

    labelAndForwardIPv4Datagram(ipdatagram);
}

bool MPLS::tryLabelAndForwardIPv4Datagram(IPv4Datagram *ipdatagram)
{
    LabelOpVector outLabel;
    std::string outInterface;
    int color;

    if (!pct->lookupLabel(ipdatagram, outLabel, outInterface, color)) {
        EV_WARN << "no mapping exists for this packet" << endl;
        return false;
    }

    ASSERT(outLabel.size() > 0);

    int outgoingPort = ift->getInterfaceByName(outInterface.c_str())->getNetworkLayerGateIndex();

    MPLSPacket *mplsPacket = new MPLSPacket(ipdatagram->getName());
    mplsPacket->encapsulate(ipdatagram);
    doStackOps(mplsPacket, outLabel);

    EV_INFO << "forwarding packet to " << outInterface << endl;

    mplsPacket->addPar("color") = color;

    if (!mplsPacket->hasLabel()) {
        // yes, this may happen - if we'are both ingress and egress
        ipdatagram = check_and_cast<IPv4Datagram *>(mplsPacket->decapsulate());    // XXX FIXME superfluous encaps/decaps
        delete mplsPacket;
        sendToL2(ipdatagram, outgoingPort);
    }
    else
        sendToL2(mplsPacket, outgoingPort);

    return true;
}

void MPLS::labelAndForwardIPv4Datagram(IPv4Datagram *ipdatagram)
{
    if (tryLabelAndForwardIPv4Datagram(ipdatagram))
        return;

    // handling our outgoing IPv4 traffic that didn't match any FEC/LSP
    // do not use labelAndForwardIPv4Datagram for packets arriving to ingress!

    EV_INFO << "FEC not resolved, doing regular L3 routing" << endl;

    int gateIndex = ipdatagram->getArrivalGate()->getIndex();

    sendToL2(ipdatagram, gateIndex);
}

void MPLS::doStackOps(MPLSPacket *mplsPacket, const LabelOpVector& outLabel)
{
    unsigned int n = outLabel.size();

    EV_INFO << "doStackOps: " << outLabel << endl;

    ASSERT(n >= 0);

    for (unsigned int i = 0; i < n; i++) {
        switch (outLabel[i].optcode) {
            case PUSH_OPER:
                mplsPacket->pushLabel(outLabel[i].label);
                break;

            case SWAP_OPER:
                ASSERT(mplsPacket->hasLabel());
                mplsPacket->swapLabel(outLabel[i].label);
                break;

            case POP_OPER:
                ASSERT(mplsPacket->hasLabel());
                mplsPacket->popLabel();
                break;

            default:
                throw cRuntimeError("Unknown MPLS OptCode %d", outLabel[i].optcode);
                break;
        }
    }
}

void MPLS::processPacketFromL2(cMessage *msg)
{
    IPv4Datagram *ipdatagram = dynamic_cast<IPv4Datagram *>(msg);
    MPLSPacket *mplsPacket = dynamic_cast<MPLSPacket *>(msg);

    if (mplsPacket) {
        processMPLSPacketFromL2(mplsPacket);
    }
    else if (ipdatagram) {
        // IPv4 datagram arrives at Ingress router. We'll try to classify it
        // and add an MPLS header

        if (!tryLabelAndForwardIPv4Datagram(ipdatagram)) {
            int gateIndex = ipdatagram->getArrivalGate()->getIndex();
            send(ipdatagram, "netwOut", gateIndex);
        }
    }
    else {
        throw cRuntimeError("Unknown message received");
    }
}

void MPLS::processMPLSPacketFromL2(MPLSPacket *mplsPacket)
{
    int gateIndex = mplsPacket->getArrivalGate()->getIndex();
    InterfaceEntry *ie = ift->getInterfaceByNetworkLayerGateIndex(gateIndex);
    std::string senderInterface = ie->getName();
    ASSERT(mplsPacket->hasLabel());
    int oldLabel = mplsPacket->getTopLabel();

    EV_INFO << "Received " << mplsPacket << " from L2, label=" << oldLabel << " inInterface=" << senderInterface << endl;

    if (oldLabel == -1) {
        // This is a IPv4 native packet (RSVP/TED traffic)
        // Decapsulate the message and pass up to L3
        EV_INFO << ": decapsulating and sending up\n";

        IPv4Datagram *ipdatagram = check_and_cast<IPv4Datagram *>(mplsPacket->decapsulate());
        delete mplsPacket;
        send(ipdatagram, "netwOut", gateIndex);
        return;
    }

    LabelOpVector outLabel;
    std::string outInterface;
    int color;

    bool found = lt->resolveLabel(senderInterface, oldLabel, outLabel, outInterface, color);
    if (!found) {
        EV_INFO << "discarding packet, incoming label not resolved" << endl;

        delete mplsPacket;
        return;
    }

    int outgoingPort = ift->getInterfaceByName(outInterface.c_str())->getNetworkLayerGateIndex();

    doStackOps(mplsPacket, outLabel);

    if (mplsPacket->hasLabel()) {
        // forward labeled packet

        EV_INFO << "forwarding packet to " << outInterface << endl;

        if (mplsPacket->hasPar("color")) {
            mplsPacket->par("color") = color;
        }
        else {
            mplsPacket->addPar("color") = color;
        }

        //ASSERT(labelIf[outgoingPort]);

        sendToL2(mplsPacket, outgoingPort);
    }
    else {
        // last label popped, decapsulate and send out IPv4 datagram

        EV_INFO << "decapsulating IPv4 datagram" << endl;

        IPv4Datagram *nativeIP = check_and_cast<IPv4Datagram *>(mplsPacket->decapsulate());
        delete mplsPacket;

        if (outgoingPort != -1) {
            sendToL2(nativeIP, outgoingPort);
        }
        else {
            send(nativeIP, "netwOut", gateIndex);
        }
    }
}

} // namespace inet

