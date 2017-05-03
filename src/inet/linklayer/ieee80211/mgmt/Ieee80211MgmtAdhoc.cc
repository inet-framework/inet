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
#include "inet/linklayer/ieee802/Ieee802LlcHeader_m.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtAdhoc.h"

namespace inet {

namespace ieee80211 {

Define_Module(Ieee80211MgmtAdhoc);

void Ieee80211MgmtAdhoc::initialize(int stage)
{
    Ieee80211MgmtBase::initialize(stage);
}

void Ieee80211MgmtAdhoc::handleTimer(cMessage *msg)
{
    ASSERT(false);
}

void Ieee80211MgmtAdhoc::handleUpperMessage(cPacket *msg)
{
    auto packet = check_and_cast<Packet *>(msg);
    encapsulate(packet);
    sendDown(packet);
}

void Ieee80211MgmtAdhoc::handleCommand(int msgkind, cObject *ctrl)
{
    throw cRuntimeError("handleCommand(): no commands supported");
}

void Ieee80211MgmtAdhoc::encapsulate(Packet *packet)
{
    auto ethTypeTag = packet->getTag<EtherTypeReq>();
    const auto& ieee802SnapHeader = std::make_shared<Ieee802SnapHeader>();
    ieee802SnapHeader->setOui(0);
    ieee802SnapHeader->setProtocolId(ethTypeTag ? ethTypeTag->getEtherType() : -1);
    packet->insertHeader(ieee802SnapHeader);
    const auto& ieee80211MacHeader = std::make_shared<Ieee80211DataFrame>();
    ieee80211MacHeader->setReceiverAddress(packet->getMandatoryTag<MacAddressReq>()->getDestAddress());
    auto userPriorityReq = packet->getTag<UserPriorityReq>();
    if (userPriorityReq != nullptr) {
        // make it a QoS frame, and set TID
        ieee80211MacHeader->setType(ST_DATA_WITH_QOS);
        ieee80211MacHeader->setChunkLength(ieee80211MacHeader->getChunkLength() + bit(QOSCONTROL_BITS));
        ieee80211MacHeader->setTid(userPriorityReq->getUserPriority());
    }
    packet->insertHeader(ieee80211MacHeader);
}

void Ieee80211MgmtAdhoc::decapsulate(Packet *packet)
{
    const auto& ieee80211MacHeader = packet->popHeader<Ieee80211DataFrame>();
    auto macAddressInd = packet->ensureTag<MacAddressInd>();
    macAddressInd->setSrcAddress(ieee80211MacHeader->getTransmitterAddress());
    macAddressInd->setDestAddress(ieee80211MacHeader->getReceiverAddress());
    int tid = ieee80211MacHeader->getTid();
    if (tid < 8)
        packet->ensureTag<UserPriorityInd>()->setUserPriority(tid); // TID values 0..7 are UP
    packet->ensureTag<InterfaceInd>()->setInterfaceId(myIface->getInterfaceId());
    const auto& ieee802SnapHeader = packet->popHeader<Ieee802SnapHeader>();
    int etherType = ieee802SnapHeader->getProtocolId();
    if (etherType != -1) {
        packet->ensureTag<EtherTypeInd>()->setEtherType(etherType);
        packet->ensureTag<DispatchProtocolReq>()->setProtocol(ProtocolGroup::ethertype.getProtocol(etherType));
        packet->ensureTag<PacketProtocolTag>()->setProtocol(ProtocolGroup::ethertype.getProtocol(etherType));
    }
}

void Ieee80211MgmtAdhoc::handleDataFrame(Packet *packet, const Ptr<Ieee80211DataFrame>& frame)
{
    decapsulate(packet);
    sendUp(packet);
}

void Ieee80211MgmtAdhoc::handleAuthenticationFrame(Packet *packet, const Ptr<Ieee80211ManagementHeader>& frame)
{
    dropManagementFrame(packet);
}

void Ieee80211MgmtAdhoc::handleDeauthenticationFrame(Packet *packet, const Ptr<Ieee80211ManagementHeader>& frame)
{
    dropManagementFrame(packet);
}

void Ieee80211MgmtAdhoc::handleAssociationRequestFrame(Packet *packet, const Ptr<Ieee80211ManagementHeader>& frame)
{
    dropManagementFrame(packet);
}

void Ieee80211MgmtAdhoc::handleAssociationResponseFrame(Packet *packet, const Ptr<Ieee80211ManagementHeader>& frame)
{
    dropManagementFrame(packet);
}

void Ieee80211MgmtAdhoc::handleReassociationRequestFrame(Packet *packet, const Ptr<Ieee80211ManagementHeader>& frame)
{
    dropManagementFrame(packet);
}

void Ieee80211MgmtAdhoc::handleReassociationResponseFrame(Packet *packet, const Ptr<Ieee80211ManagementHeader>& frame)
{
    dropManagementFrame(packet);
}

void Ieee80211MgmtAdhoc::handleDisassociationFrame(Packet *packet, const Ptr<Ieee80211ManagementHeader>& frame)
{
    dropManagementFrame(packet);
}

void Ieee80211MgmtAdhoc::handleBeaconFrame(Packet *packet, const Ptr<Ieee80211ManagementHeader>& frame)
{
    dropManagementFrame(packet);
}

void Ieee80211MgmtAdhoc::handleProbeRequestFrame(Packet *packet, const Ptr<Ieee80211ManagementHeader>& frame)
{
    dropManagementFrame(packet);
}

void Ieee80211MgmtAdhoc::handleProbeResponseFrame(Packet *packet, const Ptr<Ieee80211ManagementHeader>& frame)
{
    dropManagementFrame(packet);
}

} // namespace ieee80211

} // namespace inet

