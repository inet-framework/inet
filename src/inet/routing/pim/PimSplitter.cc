//
// Copyright (C) 2013 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 3
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//
// Authors: Veronika Rybova, Vladimir Vesely (ivesely@fit.vutbr.cz),
//          Tamas Borbely (tomi@omnetpp.org)

#include "inet/common/ModuleAccess.h"
#include "inet/common/Protocol.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/IpProtocolId_m.h"
#include "inet/networklayer/ipv4/IcmpHeader_m.h"
#include "inet/routing/pim/PimSplitter.h"

namespace inet {

Define_Module(PimSplitter);

void PimSplitter::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        ift = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        pimIft = getModuleFromPar<PimInterfaceTable>(par("pimInterfaceTableModule"), this);

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
            EV_WARN << "Received ICMP error " << msg->getName() <<  ", ignored\n";
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
        check_and_cast<Packet *>(msg)->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(&Protocol::ipv4);
        send(msg, ipOut);
    }
    else
        throw cRuntimeError("PimSplitter: received packet on the unknown gate: %s.", arrivalGate ? arrivalGate->getBaseName() : "nullptr");
}

void PimSplitter::processPIMPacket(Packet *pkt)
{
    const auto& pimPkt = pkt->peekAtFront<PimPacket>();
    (void)pimPkt;       // unused variable
    InterfaceEntry *ie = ift->getInterfaceById(pkt->getTag<InterfaceInd>()->getInterfaceId());
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
            send(pkt, pimDMOut);
            break;

        case PimInterface::SparseMode:
            EV_INFO << "Sending packet to PimSm.\n";
            send(pkt, pimSMOut);
            break;

        default:
            throw cRuntimeError("PimSplitter: PIM mode of interface '%s' is invalid.", ie->getInterfaceName());
    }
}
}    // namespace inet

