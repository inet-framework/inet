//
// Copyright (C) 2006 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtBase.h"

#include "inet/common/INETUtils.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/lifecycle/LifecycleOperation.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211HtMgmtElements.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Tag_m.h"

namespace inet {

namespace ieee80211 {

using namespace inet::physicallayer;

void Ieee80211MgmtBase::initialize(int stage)
{
    OperationalBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        mib.reference(this, "mibModule", true);
        interfaceTable.reference(this, "interfaceTableModule", true);
        myIface = getContainingNicModule(this);
        numMgmtFramesReceived = 0;
        numMgmtFramesDropped = 0;
        getContainingNicModule(this)->subscribe(modesetChangedSignal, this);
        WATCH(numMgmtFramesReceived);
        WATCH(numMgmtFramesDropped);
    }
}

void Ieee80211MgmtBase::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signalID));

    if (signalID == modesetChangedSignal) {
        modeSet = check_and_cast<physicallayer::Ieee80211ModeSet *>(obj);
        mib->updateLocalHtCapabilities(modeSet);
        supportedRates = Ieee80211SupportedRatesElement();
        extendedSupportedRates = Ieee80211ExtendedSupportedRatesElement();
        int rateIndex = 0;
        int extendedRateIndex = 0;
        // Supported Rates carries the legacy OperationalRateSet only. HT/VHT
        // MCS support is advertised through the corresponding capabilities
        // elements (IEEE Std 802.11-2024, 9.4.2.3, 9.4.2.54.4, 11.1.4.6).
        for (const auto *mode : modeSet->getLegacyOperationalModes()) {
            bool isBasicRate = modeSet->getIsMandatory(mode);
            double rate = mode->getDataMode()->getNetBitrate().get<Mbps>();
            if (rateIndex < 8) {
                supportedRates.rate[rateIndex] = rate;
                supportedRates.basicRate[rateIndex] = isBasicRate;
                rateIndex++;
            }
            else if (extendedRateIndex < 255) {
                extendedSupportedRates.rate[extendedRateIndex] = rate;
                extendedSupportedRates.basicRate[extendedRateIndex] = isBasicRate;
                extendedRateIndex++;
            }
            else
                throw cRuntimeError("Mode set '%s' contains more than 263 legacy operational rates", modeSet->getName());
        }
        supportedRates.numRates = rateIndex;
        extendedSupportedRates.numRates = extendedRateIndex;
    }
}

void Ieee80211MgmtBase::addHtCapabilities(const Ptr<Ieee80211MgmtFrame>& frame) const
{
    if (mib->isHtOperationSupported())
        setHtCapabilities(frame, mib->localHtCapabilities);
}

void Ieee80211MgmtBase::addHtOperation(const Ptr<Ieee80211MgmtFrame>& frame) const
{
    if (mib->isHtOperationSupported())
        setHtOperation(frame, mib->getHtOperation());
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
    mib->clearPeerHtCapabilities();
}

} // namespace ieee80211

} // namespace inet
