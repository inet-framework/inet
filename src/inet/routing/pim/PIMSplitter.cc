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

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/Protocol.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/networklayer/common/IPProtocolId_m.h"
#include "inet/networklayer/contract/ipv4/IPv4ControlInfo.h"
#include "inet/networklayer/ipv4/ICMPMessage_m.h"
#include "inet/common/ModuleAccess.h"
#include "inet/routing/pim/PIMSplitter.h"

namespace inet {
using namespace std;

Define_Module(PIMSplitter);

void PIMSplitter::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        ift = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        pimIft = getModuleFromPar<PIMInterfaceTable>(par("pimInterfaceTableModule"), this);

        ipIn = gate("ipIn");
        ipOut = gate("ipOut");
        pimDMIn = gate("pimDMIn");
        pimDMOut = gate("pimDMOut");
        pimSMIn = gate("pimSMIn");
        pimSMOut = gate("pimSMOut");
    }
    else if (stage == INITSTAGE_ROUTING_PROTOCOLS)
        registerProtocol(Protocol::pim, gate("ipOut"));
}

void PIMSplitter::handleMessage(cMessage *msg)
{
    cGate *arrivalGate = msg->getArrivalGate();

    if (arrivalGate == ipIn) {
        if (dynamic_cast<ICMPMessage *>(msg)) {
            EV_WARN << "Received ICMP error " << msg->getName() << "(" << msg->getClassName() << "), ignored\n";
            delete msg;
        }
        else if (PIMPacket *pimPacket = dynamic_cast<PIMPacket *>(msg)) {
            processPIMPacket(pimPacket);
        }
        else
            throw cRuntimeError("PIMSplitter: received unknown packet '%s (%s)' from the network layer.", msg->getName(), msg->getClassName());
    }
    else if (arrivalGate == pimSMIn || arrivalGate == pimDMIn) {
        // Send other packets to the network layer
        EV_INFO << "Received packet from PIM module, sending it to the network." << endl;
        msg->ensureTag<ProtocolReq>()->setProtocol(&Protocol::ipv4);
        send(msg, ipOut);
    }
    else
        throw cRuntimeError("PIMSplitter: received packet on the unknown gate: %s.", arrivalGate ? arrivalGate->getBaseName() : "nullptr");
}

void PIMSplitter::processPIMPacket(PIMPacket *pkt)
{
    IPv4ControlInfo *ctrlInfo = check_and_cast<IPv4ControlInfo *>(pkt->getControlInfo());
    InterfaceEntry *ie = ift->getInterfaceById(ctrlInfo->getInterfaceId());
    ASSERT(ie);

    EV_INFO << "Received packet on interface '" << ie->getName() << "'" << endl;

    PIMInterface *pimInt = pimIft->getInterfaceById(ie->getInterfaceId());
    if (!pimInt) {
        EV_WARN << "PIM is not enabled on interface '" << ie->getName() << "', dropping packet." << endl;
        delete pkt;
        return;
    }

    switch (pimInt->getMode()) {
        case PIMInterface::DenseMode:
            EV_INFO << "Sending packet to PIMDM.\n";
            send(pkt, pimDMOut);
            break;

        case PIMInterface::SparseMode:
            EV_INFO << "Sending packet to PIMSM.\n";
            send(pkt, pimSMOut);
            break;

        default:
            throw cRuntimeError("PIMSplitter: PIM mode of interface '%s' is invalid.", ie->getName());
    }
}
}    // namespace inet

