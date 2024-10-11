//
// Copyright (C) 2013 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

// Authors: Veronika Rybova, Vladimir Vesely (ivesely@fit.vutbr.cz),
//          Tamas Borbely (tomi@omnetpp.org)

#include "inet/routing/pim/PimSplitter.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/Protocol.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/IpProtocolId_m.h"
#include "inet/networklayer/ipv4/IcmpHeader_m.h"

namespace inet {

Define_Module(PimSplitter);

void PimSplitter::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        ift.reference(this, "interfaceTableModule", true);
        pimIft.reference(this, "pimInterfaceTableModule", true);
        ipSink.reference(gate("ipOut"), true);
        DispatchProtocolReq dispatchProtocolReq;
        dispatchProtocolReq.setProtocol(&Protocol::pim);
        dispatchProtocolReq.setServicePrimitive(SP_INDICATION);
        pimDMSink.reference(gate("pimDMOut"), true, &dispatchProtocolReq);
        pimSMSink.reference(gate("pimSMOut"), true, &dispatchProtocolReq);

        ipIn = gate("ipIn");
        ipOut = gate("ipOut");
        pimDMIn = gate("pimDMIn");
        pimDMOut = gate("pimDMOut");
        pimSMIn = gate("pimSMIn");
        pimSMOut = gate("pimSMOut");
    }
}

void PimSplitter::handleMessage(cMessage *msg)
{
    cGate *arrivalGate = msg->getArrivalGate();

    if (arrivalGate == ipIn) {
        Packet *packet = check_and_cast<Packet *>(msg);
        auto protocol = packet->getTag<PacketProtocolTag>()->getProtocol();
        if (protocol == &Protocol::icmpv4) {
            EV_WARN << "Received ICMP error " << msg->getName() << ", ignored\n";
            delete msg;
        }
        else if (protocol == &Protocol::pim) {
            processPIMPacket(packet);
        }
        else
            throw cRuntimeError("PimSplitter: received unknown packet '%s (%s)' from the network layer.", msg->getName(), msg->getClassName());
    }
    else if (arrivalGate == pimSMIn || arrivalGate == pimDMIn) {
        // Send other packets to the network layer
        EV_INFO << "Received packet from PIM module, sending it to the network." << endl;
        auto packet = check_and_cast<Packet *>(msg);
        packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(&Protocol::ipv4);
        ipSink.pushPacket(packet);
    }
    else
        throw cRuntimeError("PimSplitter: received packet on the unknown gate: %s.", arrivalGate ? arrivalGate->getBaseName() : "nullptr");
}

void PimSplitter::processPIMPacket(Packet *pkt)
{
    const auto& pimPkt = pkt->peekAtFront<PimPacket>();
    (void)pimPkt; // unused variable
    NetworkInterface *ie = ift->getInterfaceById(pkt->getTag<InterfaceInd>()->getInterfaceId());
    ASSERT(ie);

    EV_INFO << "Received packet on interface '" << ie->getInterfaceName() << "'" << endl;

    PimInterface *pimInt = pimIft->getInterfaceById(ie->getInterfaceId());
    if (!pimInt) {
        EV_WARN << "PIM is not enabled on interface '" << ie->getInterfaceName() << "', dropping packet." << endl;
        delete pkt;
        return;
    }

    switch (pimInt->getMode()) {
        case PimInterface::DenseMode:
            EV_INFO << "Sending packet to PimDm.\n";
            pimDMSink.pushPacket(pkt);
            break;

        case PimInterface::SparseMode:
            EV_INFO << "Sending packet to PimSm.\n";
            pimSMSink.pushPacket(pkt);
            break;

        default:
            throw cRuntimeError("PimSplitter: PIM mode of interface '%s' is invalid.", ie->getInterfaceName());
    }
}

void PimSplitter::pushPacket(Packet *packet, const cGate *gate)
{
    Enter_Method("pushPacket");
    take(packet);
    packet->setArrival(getId(), gate->getId());
    handleMessage(packet);
}

} // namespace inet

