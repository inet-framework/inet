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

#include "inet/common/INETUtils.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/lifecycle/LifecycleOperation.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtBase.h"
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211Tag_m.h"

namespace inet {

namespace ieee80211 {

using namespace inet::physicallayer;

void Ieee80211MgmtBase::initialize(int stage)
{
    OperationalBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        mib = getModuleFromPar<Ieee80211Mib>(par("mibModule"), this);
        interfaceTable = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        myIface = getContainingNicModule(this);
        numMgmtFramesReceived = 0;
        numMgmtFramesDropped = 0;
        WATCH(numMgmtFramesReceived);
        WATCH(numMgmtFramesDropped);
    }
}

void Ieee80211MgmtBase::handleMessageWhenUp(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        // process timers
        EV << "Timer expired: " << msg << "\n";
        handleTimer(msg);
    }
    else if (msg->arrivedOn("macIn")) {
        // process incoming frame
        EV << "Frame arrived from MAC: " << msg << "\n";
        auto packet = check_and_cast<Packet *>(msg);
        const Ptr<const Ieee80211DataOrMgmtHeader>& header = packet->peekAt<Ieee80211DataOrMgmtHeader>(packet->getFrontOffset() - B(24));
        processFrame(packet, header);
    }
    else if (msg->arrivedOn("agentIn")) {
        // process command from agent
        EV << "Command arrived from agent: " << msg << "\n";
        int msgkind = msg->getKind();
        cObject *ctrl = msg->removeControlInfo();
        delete msg;

        handleCommand(msgkind, ctrl);
    }
    else
        throw cRuntimeError("Unknown message");
}

void Ieee80211MgmtBase::sendDown(Packet *frame)
{
    ASSERT(isUp());
    frame->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::ieee80211Mgmt);
    send(frame, "macOut");
}

void Ieee80211MgmtBase::dropManagementFrame(Packet *frame)
{
    EV << "ignoring management frame: " << (cMessage *)frame << "\n";
    delete frame;
    numMgmtFramesDropped++;
}

void Ieee80211MgmtBase::processFrame(Packet *packet, const Ptr<const Ieee80211DataOrMgmtHeader>& header)
{
    switch (header->getType()) {
        case ST_AUTHENTICATION:
            numMgmtFramesReceived++;
            handleAuthenticationFrame(packet, dynamicPtrCast<const Ieee80211MgmtHeader>(header));
            break;

        case ST_DEAUTHENTICATION:
            numMgmtFramesReceived++;
            handleDeauthenticationFrame(packet, dynamicPtrCast<const Ieee80211MgmtHeader>(header));
            break;

        case ST_ASSOCIATIONREQUEST:
            numMgmtFramesReceived++;
            handleAssociationRequestFrame(packet, dynamicPtrCast<const Ieee80211MgmtHeader>(header));
            break;

        case ST_ASSOCIATIONRESPONSE:
            numMgmtFramesReceived++;
            handleAssociationResponseFrame(packet, dynamicPtrCast<const Ieee80211MgmtHeader>(header));
            break;

        case ST_REASSOCIATIONREQUEST:
            numMgmtFramesReceived++;
            handleReassociationRequestFrame(packet, dynamicPtrCast<const Ieee80211MgmtHeader>(header));
            break;

        case ST_REASSOCIATIONRESPONSE:
            numMgmtFramesReceived++;
            handleReassociationResponseFrame(packet, dynamicPtrCast<const Ieee80211MgmtHeader>(header));
            break;

        case ST_DISASSOCIATION:
            numMgmtFramesReceived++;
            handleDisassociationFrame(packet, dynamicPtrCast<const Ieee80211MgmtHeader>(header));
            break;

        case ST_BEACON:
            numMgmtFramesReceived++;
            handleBeaconFrame(packet, dynamicPtrCast<const Ieee80211MgmtHeader>(header));
            break;

        case ST_PROBEREQUEST:
            numMgmtFramesReceived++;
            handleProbeRequestFrame(packet, dynamicPtrCast<const Ieee80211MgmtHeader>(header));
            break;

        case ST_PROBERESPONSE:
            numMgmtFramesReceived++;
            handleProbeResponseFrame(packet, dynamicPtrCast<const Ieee80211MgmtHeader>(header));
            break;

        default:
            throw cRuntimeError("Unexpected frame type (%s)%s", packet->getClassName(), packet->getName());
    }
}

void Ieee80211MgmtBase::start()
{
}

void Ieee80211MgmtBase::stop()
{
}

} // namespace ieee80211

} // namespace inet

