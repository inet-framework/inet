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
    Ieee80211DataFrame *frame = encapsulate(msg);
    sendDown(frame);
}

void Ieee80211MgmtAdhoc::handleCommand(int msgkind, cObject *ctrl)
{
    throw cRuntimeError("handleCommand(): no commands supported");
}

Ieee80211DataFrame *Ieee80211MgmtAdhoc::encapsulate(cPacket *msg)
{
    Ieee80211DataFrameWithSNAP *frame = new Ieee80211DataFrameWithSNAP(msg->getName());

    // copy receiver address from the control info (sender address will be set in MAC)
    frame->setReceiverAddress(msg->getMandatoryTag<MacAddressReq>()->getDestAddress());
    auto ethTypeTag = msg->getTag<EtherTypeReq>();
    frame->setEtherType(ethTypeTag ? ethTypeTag->getEtherType() : -1);
    auto userPriorityReq = msg->getTag<UserPriorityReq>();
    if (userPriorityReq != nullptr) {
        // make it a QoS frame, and set TID
        frame->setType(ST_DATA_WITH_QOS);
        frame->addBitLength(QOSCONTROL_BITS);
        frame->setTid(userPriorityReq->getUserPriority());
    }

    frame->encapsulate(msg);
    return frame;
}

cPacket *Ieee80211MgmtAdhoc::decapsulate(Ieee80211DataFrame *frame)
{
    cPacket *payload = frame->decapsulate();

    auto macAddressInd = payload->ensureTag<MacAddressInd>();
    macAddressInd->setSrcAddress(frame->getTransmitterAddress());
    macAddressInd->setDestAddress(frame->getReceiverAddress());
    int tid = frame->getTid();
    if (tid < 8)
        payload->ensureTag<UserPriorityInd>()->setUserPriority(tid); // TID values 0..7 are UP
    payload->ensureTag<InterfaceInd>()->setInterfaceId(myIface->getInterfaceId());
    Ieee80211DataFrameWithSNAP *frameWithSNAP = dynamic_cast<Ieee80211DataFrameWithSNAP *>(frame);
    if (frameWithSNAP) {
        int etherType = frameWithSNAP->getEtherType();
        if (etherType != -1) {
            payload->ensureTag<EtherTypeInd>()->setEtherType(etherType);
            payload->ensureTag<DispatchProtocolReq>()->setProtocol(ProtocolGroup::ethertype.getProtocol(etherType));
        }
    }

    delete frame;
    return payload;
}

void Ieee80211MgmtAdhoc::handleDataFrame(Ieee80211DataFrame *frame)
{
    sendUp(decapsulate(frame));
}

void Ieee80211MgmtAdhoc::handleAuthenticationFrame(Ieee80211AuthenticationFrame *frame)
{
    dropManagementFrame(frame);
}

void Ieee80211MgmtAdhoc::handleDeauthenticationFrame(Ieee80211DeauthenticationFrame *frame)
{
    dropManagementFrame(frame);
}

void Ieee80211MgmtAdhoc::handleAssociationRequestFrame(Ieee80211AssociationRequestFrame *frame)
{
    dropManagementFrame(frame);
}

void Ieee80211MgmtAdhoc::handleAssociationResponseFrame(Ieee80211AssociationResponseFrame *frame)
{
    dropManagementFrame(frame);
}

void Ieee80211MgmtAdhoc::handleReassociationRequestFrame(Ieee80211ReassociationRequestFrame *frame)
{
    dropManagementFrame(frame);
}

void Ieee80211MgmtAdhoc::handleReassociationResponseFrame(Ieee80211ReassociationResponseFrame *frame)
{
    dropManagementFrame(frame);
}

void Ieee80211MgmtAdhoc::handleDisassociationFrame(Ieee80211DisassociationFrame *frame)
{
    dropManagementFrame(frame);
}

void Ieee80211MgmtAdhoc::handleBeaconFrame(Ieee80211BeaconFrame *frame)
{
    dropManagementFrame(frame);
}

void Ieee80211MgmtAdhoc::handleProbeRequestFrame(Ieee80211ProbeRequestFrame *frame)
{
    dropManagementFrame(frame);
}

void Ieee80211MgmtAdhoc::handleProbeResponseFrame(Ieee80211ProbeResponseFrame *frame)
{
    dropManagementFrame(frame);
}

} // namespace ieee80211

} // namespace inet

