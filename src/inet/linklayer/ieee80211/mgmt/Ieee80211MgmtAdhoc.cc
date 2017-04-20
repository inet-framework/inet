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

#include "inet/linklayer/common/UserPriorityTag_m.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/EtherTypeTag_m.h"
#include "inet/linklayer/common/Ieee802Ctrl.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/MACAddressTag_m.h"
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

void Ieee80211MgmtAdhoc::encapsulate(Packet *msg)
{
    const Ptr<Ieee80211DataFrameWithSNAP>& frame = std::make_shared<Ieee80211DataFrameWithSNAP>();

    // copy receiver address from the control info (sender address will be set in MAC)
    frame->setReceiverAddress(msg->getMandatoryTag<MacAddressReq>()->getDestAddress());
    auto ethTypeTag = msg->getTag<EtherTypeReq>();
    frame->setEtherType(ethTypeTag ? ethTypeTag->getEtherType() : -1);
    auto userPriorityReq = msg->getTag<UserPriorityReq>();
    if (userPriorityReq != nullptr) {
        // make it a QoS frame, and set TID
        frame->setType(ST_DATA_WITH_QOS);
        frame->setChunkLength(frame->getChunkLength() + bit(QOSCONTROL_BITS));
        frame->setTid(userPriorityReq->getUserPriority());
    }

    msg->insertHeader(frame);
}

void Ieee80211MgmtAdhoc::decapsulate(Packet *packet)
{
    const auto& frame = packet->popHeader<Ieee80211DataFrame>();
    auto macAddressInd = packet->ensureTag<MacAddressInd>();
    macAddressInd->setSrcAddress(frame->getTransmitterAddress());
    macAddressInd->setDestAddress(frame->getReceiverAddress());
    int tid = frame->getTid();
    if (tid < 8)
        packet->ensureTag<UserPriorityInd>()->setUserPriority(tid); // TID values 0..7 are UP
    packet->ensureTag<InterfaceInd>()->setInterfaceId(myIface->getInterfaceId());
    const Ptr<Ieee80211DataFrameWithSNAP>& frameWithSNAP = std::dynamic_pointer_cast<Ieee80211DataFrameWithSNAP>(frame);
    if (frameWithSNAP) {
        int etherType = frameWithSNAP->getEtherType();
        if (etherType != -1) {
            packet->ensureTag<EtherTypeInd>()->setEtherType(etherType);
            packet->ensureTag<DispatchProtocolReq>()->setProtocol(ProtocolGroup::ethertype.getProtocol(etherType));
            packet->ensureTag<PacketProtocolTag>()->setProtocol(ProtocolGroup::ethertype.getProtocol(etherType));
        }
    }
}

void Ieee80211MgmtAdhoc::handleDataFrame(Packet *packet, const Ptr<Ieee80211DataFrame>& frame)
{
    decapsulate(packet);
    sendUp(packet);
}

void Ieee80211MgmtAdhoc::handleAuthenticationFrame(Packet *packet, const Ptr<Ieee80211AuthenticationFrame>& frame)
{
    dropManagementFrame(packet);
}

void Ieee80211MgmtAdhoc::handleDeauthenticationFrame(Packet *packet, const Ptr<Ieee80211DeauthenticationFrame>& frame)
{
    dropManagementFrame(packet);
}

void Ieee80211MgmtAdhoc::handleAssociationRequestFrame(Packet *packet, const Ptr<Ieee80211AssociationRequestFrame>& frame)
{
    dropManagementFrame(packet);
}

void Ieee80211MgmtAdhoc::handleAssociationResponseFrame(Packet *packet, const Ptr<Ieee80211AssociationResponseFrame>& frame)
{
    dropManagementFrame(packet);
}

void Ieee80211MgmtAdhoc::handleReassociationRequestFrame(Packet *packet, const Ptr<Ieee80211ReassociationRequestFrame>& frame)
{
    dropManagementFrame(packet);
}

void Ieee80211MgmtAdhoc::handleReassociationResponseFrame(Packet *packet, const Ptr<Ieee80211ReassociationResponseFrame>& frame)
{
    dropManagementFrame(packet);
}

void Ieee80211MgmtAdhoc::handleDisassociationFrame(Packet *packet, const Ptr<Ieee80211DisassociationFrame>& frame)
{
    dropManagementFrame(packet);
}

void Ieee80211MgmtAdhoc::handleBeaconFrame(Packet *packet, const Ptr<Ieee80211BeaconFrame>& frame)
{
    dropManagementFrame(packet);
}

void Ieee80211MgmtAdhoc::handleProbeRequestFrame(Packet *packet, const Ptr<Ieee80211ProbeRequestFrame>& frame)
{
    dropManagementFrame(packet);
}

void Ieee80211MgmtAdhoc::handleProbeResponseFrame(Packet *packet, const Ptr<Ieee80211ProbeResponseFrame>& frame)
{
    dropManagementFrame(packet);
}

} // namespace ieee80211

} // namespace inet

