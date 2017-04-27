//
// Copyright (C) 2006 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
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

#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/EtherTypeTag_m.h"
#include "inet/linklayer/common/Ieee802Ctrl.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/MACAddressTag_m.h"
#include "inet/linklayer/common/UserPriorityTag_m.h"
#include "inet/linklayer/ieee802/Ieee802Header_m.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtSTASimplified.h"

namespace inet {

namespace ieee80211 {

Define_Module(Ieee80211MgmtSTASimplified);

void Ieee80211MgmtSTASimplified::initialize(int stage)
{
    Ieee80211MgmtBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        accessPointAddress.setAddress(par("accessPointAddress").stringValue());
        receiveSequence = 0;
    }
}

void Ieee80211MgmtSTASimplified::handleTimer(cMessage *msg)
{
    ASSERT(false);
}

void Ieee80211MgmtSTASimplified::handleUpperMessage(cPacket *msg)
{
    if (accessPointAddress.isUnspecified()) {
        EV << "STA is not associated with an access point, discarding packet " << msg << "\n";
        delete msg;
        return;
    }
    auto packet = check_and_cast<Packet *>(msg);
    encapsulate(packet);
    sendDown(packet);
}

void Ieee80211MgmtSTASimplified::handleCommand(int msgkind, cObject *ctrl)
{
    throw cRuntimeError("handleCommand(): no commands supported");
}

void Ieee80211MgmtSTASimplified::encapsulate(Packet *packet)
{
    auto ethTypeTag = packet->getTag<EtherTypeReq>();
    const auto& ieee802SnapHeader = std::make_shared<Ieee802SnapHeader>();
    ieee802SnapHeader->setOui(0);
    ieee802SnapHeader->setProtocolId(ethTypeTag ? ethTypeTag->getEtherType() : -1);
    packet->insertHeader(ieee802SnapHeader);

    auto ieee80211MacHeader = std::make_shared<Ieee80211DataFrame>();
    // frame goes to the AP
    ieee80211MacHeader->setToDS(true);
    // receiver is the AP
    ieee80211MacHeader->setReceiverAddress(accessPointAddress);

    // destination address is in address3
    MACAddress dest = packet->getMandatoryTag<MacAddressReq>()->getDestAddress();
    ASSERT(!dest.isUnspecified());
    ieee80211MacHeader->setAddress3(dest);
    auto userPriorityReq = packet->getTag<UserPriorityReq>();
    if (userPriorityReq != nullptr) {
        // make it a QoS ieee80211MacHeader, and set TID
        ieee80211MacHeader->setType(ST_DATA_WITH_QOS);
        ieee80211MacHeader->setChunkLength(ieee80211MacHeader->getChunkLength() + bit(QOSCONTROL_BITS));
        ieee80211MacHeader->setTid(userPriorityReq->getUserPriority());
    }

    packet->insertHeader(ieee80211MacHeader);
    packet->insertTrailer(std::make_shared<Ieee80211MacTrailer>());
}

void Ieee80211MgmtSTASimplified::decapsulate(Packet *packet)
{
    const auto& ieee80211MacHeader = packet->popHeader<Ieee80211DataFrame>();
    auto macAddressInd = packet->ensureTag<MacAddressInd>();
    macAddressInd->setSrcAddress(ieee80211MacHeader->getAddress3());
    macAddressInd->setDestAddress(ieee80211MacHeader->getReceiverAddress());
    if (ieee80211MacHeader->getType() == ST_DATA_WITH_QOS) {
        int tid = ieee80211MacHeader->getTid();
        if (tid < 8)
            packet->ensureTag<UserPriorityInd>()->setUserPriority(tid); // TID values 0..7 are UP
    }
    packet->ensureTag<InterfaceInd>()->setInterfaceId(myIface->getInterfaceId());
    const auto& ieee802SnapHeader = packet->popHeader<Ieee802SnapHeader>();
    int etherType = ieee802SnapHeader->getProtocolId();
    if (etherType != -1) {
        packet->ensureTag<EtherTypeInd>()->setEtherType(etherType);
        packet->ensureTag<DispatchProtocolReq>()->setProtocol(ProtocolGroup::ethertype.getProtocol(etherType));
        packet->ensureTag<PacketProtocolTag>()->setProtocol(ProtocolGroup::ethertype.getProtocol(etherType));
    }
    packet->popTrailer<Ieee80211MacTrailer>();
}

void Ieee80211MgmtSTASimplified::handleDataFrame(Packet *packet, const Ptr<Ieee80211DataFrame>& frame)
{
    decapsulate(packet);
    sendUp(packet);
}

void Ieee80211MgmtSTASimplified::handleAuthenticationFrame(Packet *packet, const Ptr<Ieee80211AuthenticationFrame>& frame)
{
    dropManagementFrame(packet);
}

void Ieee80211MgmtSTASimplified::handleDeauthenticationFrame(Packet *packet, const Ptr<Ieee80211DeauthenticationFrame>& frame)
{
    dropManagementFrame(packet);
}

void Ieee80211MgmtSTASimplified::handleAssociationRequestFrame(Packet *packet, const Ptr<Ieee80211AssociationRequestFrame>& frame)
{
    dropManagementFrame(packet);
}

void Ieee80211MgmtSTASimplified::handleAssociationResponseFrame(Packet *packet, const Ptr<Ieee80211AssociationResponseFrame>& frame)
{
    dropManagementFrame(packet);
}

void Ieee80211MgmtSTASimplified::handleReassociationRequestFrame(Packet *packet, const Ptr<Ieee80211ReassociationRequestFrame>& frame)
{
    dropManagementFrame(packet);
}

void Ieee80211MgmtSTASimplified::handleReassociationResponseFrame(Packet *packet, const Ptr<Ieee80211ReassociationResponseFrame>& frame)
{
    dropManagementFrame(packet);
}

void Ieee80211MgmtSTASimplified::handleDisassociationFrame(Packet *packet, const Ptr<Ieee80211DisassociationFrame>& frame)
{
    dropManagementFrame(packet);
}

void Ieee80211MgmtSTASimplified::handleBeaconFrame(Packet *packet, const Ptr<Ieee80211BeaconFrame>& frame)
{
    dropManagementFrame(packet);
}

void Ieee80211MgmtSTASimplified::handleProbeRequestFrame(Packet *packet, const Ptr<Ieee80211ProbeRequestFrame>& frame)
{
    dropManagementFrame(packet);
}

void Ieee80211MgmtSTASimplified::handleProbeResponseFrame(Packet *packet, const Ptr<Ieee80211ProbeResponseFrame>& frame)
{
    dropManagementFrame(packet);
}

} // namespace ieee80211

} // namespace inet

