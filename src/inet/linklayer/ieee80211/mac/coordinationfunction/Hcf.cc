//
// Copyright (C) 2016 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#include "inet/common/ModuleAccess.h"
#include "inet/linklayer/ieee8022/Ieee8022LlcHeader_m.h"
#include "inet/linklayer/ieee80211/mac/blockack/OriginatorBlockAckAgreementHandler.h"
#include "inet/linklayer/ieee80211/mac/blockack/OriginatorBlockAckProcedure.h"
#include "inet/linklayer/ieee80211/mac/blockack/RecipientBlockAckAgreementHandler.h"
#include "inet/linklayer/ieee80211/mac/coordinationfunction/Hcf.h"
#include "inet/linklayer/ieee80211/mac/framesequence/HcfFs.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Mac.h"
#include "inet/linklayer/ieee80211/mac/recipient/RecipientAckProcedure.h"
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211Tag_m.h"

namespace inet {
namespace ieee80211 {

Define_Module(Hcf);

void Hcf::initialize(int stage)
{
    ModeSetListener::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        mac = check_and_cast<Ieee80211Mac *>(getContainingNicModule(this)->getSubmodule("mac"));
        startRxTimer = new cMessage("startRxTimeout");
        inactivityTimer = new cMessage("blockAckInactivityTimer");
        numEdcafs = par("numEdcafs");
        edca = check_and_cast<Edca *>(getSubmodule("edca"));
        hcca = check_and_cast<Hcca *>(getSubmodule("hcca"));
        tx = check_and_cast<ITx *>(getModuleByPath(par("txModule")));
        rx = check_and_cast<IRx *>(getModuleByPath(par("rxModule")));
        dataAndMgmtRateControl = dynamic_cast<IRateControl*>(getSubmodule("rateControl"));
        originatorBlockAckAgreementPolicy = dynamic_cast<IOriginatorBlockAckAgreementPolicy*>(getSubmodule("originatorBlockAckAgreementPolicy"));
        recipientBlockAckAgreementPolicy = dynamic_cast<IRecipientBlockAckAgreementPolicy*>(getSubmodule("recipientBlockAckAgreementPolicy"));
        rateSelection = check_and_cast<IQoSRateSelection *>(getSubmodule("rateSelection"));
        frameSequenceHandler = new FrameSequenceHandler();
        originatorDataService = check_and_cast<IOriginatorMacDataService *>(getSubmodule(("originatorMacDataService")));
        recipientDataService = check_and_cast<IRecipientQoSMacDataService*>(getSubmodule("recipientMacDataService"));
        originatorAckPolicy = check_and_cast<IOriginatorQoSAckPolicy*>(getSubmodule("originatorAckPolicy"));
        recipientAckPolicy = check_and_cast<IRecipientQoSAckPolicy*>(getSubmodule("recipientAckPolicy"));
        edcaMgmtAndNonQoSRecoveryProcedure = check_and_cast<NonQoSRecoveryProcedure *>(getSubmodule("edcaMgmtAndNonQoSRecoveryProcedure"));
        singleProtectionMechanism = check_and_cast<SingleProtectionMechanism*>(getSubmodule("singleProtectionMechanism"));
        rtsProcedure = new RtsProcedure();
        rtsPolicy = check_and_cast<IRtsPolicy*>(getSubmodule("rtsPolicy"));
        recipientAckProcedure = new RecipientAckProcedure();
        ctsProcedure = new CtsProcedure();
        ctsPolicy = check_and_cast<ICtsPolicy*>(getSubmodule("ctsPolicy"));
        if (originatorBlockAckAgreementPolicy && recipientBlockAckAgreementPolicy) {
            recipientBlockAckAgreementHandler = new RecipientBlockAckAgreementHandler();
            originatorBlockAckAgreementHandler = new OriginatorBlockAckAgreementHandler();
            originatorBlockAckProcedure = new OriginatorBlockAckProcedure();
            recipientBlockAckProcedure = new RecipientBlockAckProcedure();
        }
        for (int ac = 0; ac < numEdcafs; ac++) {
            edcaPendingQueues.push_back(new PendingQueue(par("maxQueueSize"), nullptr, par("prioritizeMulticast") ? PendingQueue::Priority::PRIORITIZE_MULTICAST_OVER_DATA : PendingQueue::Priority::PRIORITIZE_MGMT_OVER_DATA));
            edcaDataRecoveryProcedures.push_back(check_and_cast<QoSRecoveryProcedure *>(getSubmodule("edcaDataRecoveryProcedures", ac)));
            edcaAckHandlers.push_back(new QoSAckHandler());
            edcaInProgressFrames.push_back(new InProgressFrames(edcaPendingQueues[ac], originatorDataService, edcaAckHandlers[ac]));
            edcaTxops.push_back(check_and_cast<TxopProcedure *>(getSubmodule("edcaTxopProcedures", ac)));
            stationRetryCounters.push_back(new StationRetryCounters());
        }
    }
}

void Hcf::handleMessage(cMessage* msg)
{
    if (msg == startRxTimer) {
        if (!isReceptionInProgress()) {
            frameSequenceHandler->handleStartRxTimeout();
            updateDisplayString();
        }
    }
    else if (msg == inactivityTimer) {
        if (originatorBlockAckAgreementHandler && recipientBlockAckAgreementHandler) {
            originatorBlockAckAgreementHandler->blockAckAgreementExpired(this, this);
            recipientBlockAckAgreementHandler->blockAckAgreementExpired(this, this);
        }
        else
            throw cRuntimeError("Unknown event");
    }
    else
        throw cRuntimeError("Unknown msg type");
}

void Hcf::updateDisplayString()
{
    if (frameSequenceHandler->isSequenceRunning()) {
        auto history = frameSequenceHandler->getFrameSequence()->getHistory();
        if (history.length() > 32) {
            history.erase(history.begin(), history.end() - 32);
            history = "..." + history;
        }
        getDisplayString().setTagArg("t", 0, history.c_str());
    }
    else
        getDisplayString().removeTag("t");
}

void Hcf::processUpperFrame(Packet *packet, const Ptr<Ieee80211DataOrMgmtHeader>& frame)
{
    Enter_Method("processUpperFrame(%s)", frame->getName());
    EV_INFO << "Processing upper frame: " << frame->getName() << endl;
    // TODO:
    // A QoS STA should send individually addressed Management frames that are addressed to a non-QoS STA
    // using the access category AC_BE and shall send all other management frames using the access category
    // AC_VO. A QoS STA that does not send individually addressed Management frames that are addressed to a
    // non-QoS STA using the access category AC_BE shall send them using the access category AC_VO.
    // Management frames are exempted from any and all restrictions on transmissions arising from admission
    // control procedures.
    AccessCategory ac = AccessCategory(-1);
    if (std::dynamic_pointer_cast<Ieee80211MgmtHeader>(frame)) // TODO: + non-QoS frames
        ac = AccessCategory::AC_VO;
    else if (auto dataFrame = std::dynamic_pointer_cast<Ieee80211DataHeader>(frame))
        ac = edca->classifyFrame(dataFrame);
    else
        throw cRuntimeError("Unknown message type");
    EV_INFO << "The upper frame has been classified as a " << printAccessCategory(ac) << " frame." << endl;
    if (edcaPendingQueues[ac]->insert(packet)) {
        EV_INFO << "Frame " << frame->getName() << " has been inserted into the PendingQueue." << endl;
        auto edcaf = edca->getChannelOwner();
        if (edcaf == nullptr || edcaf->getAccessCategory() != ac) {
            EV_DETAIL << "Requesting channel for access category " << printAccessCategory(ac) << endl;
            edca->requestChannelAccess(ac, this);
        }
    }
    else {
        EV_INFO << "Frame " << frame->getName() << " has been dropped because the PendingQueue is full." << endl;
        emit(NF_PACKET_DROP, packet);
        delete packet;
    }
}

void Hcf::scheduleStartRxTimer(simtime_t timeout)
{
    Enter_Method_Silent();
    scheduleAt(simTime() + timeout, startRxTimer);
}

void Hcf::scheduleInactivityTimer(simtime_t timeout)
{
    Enter_Method_Silent();
    if (inactivityTimer->isScheduled())
        cancelEvent(inactivityTimer);
    scheduleAt(simTime() + timeout, inactivityTimer);
}

void Hcf::processLowerFrame(Packet *packet, const Ptr<Ieee80211MacHeader>& frame)
{
    Enter_Method_Silent();
    auto edcaf = edca->getChannelOwner();
    if (edcaf && frameSequenceHandler->isSequenceRunning()) {
        // TODO: always call processResponse?
        if ((!isForUs(frame) && !startRxTimer->isScheduled()) || isForUs(frame)) {
            frameSequenceHandler->processResponse(packet);
            updateDisplayString();
        }
        else {
            EV_INFO << "This frame is not for us" << std::endl;
            delete packet;
        }
        cancelEvent(startRxTimer);
    }
    else if (hcca->isOwning())
        throw cRuntimeError("Hcca is unimplemented!");
    else if (isForUs(frame))
        recipientProcessReceivedFrame(packet, frame);
    else {
        EV_INFO << "This frame is not for us" << std::endl;
        delete packet;
    }
}

void Hcf::channelGranted(IChannelAccess* channelAccess)
{
    auto edcaf = check_and_cast<Edcaf*>(channelAccess);
    if (edcaf) {
        AccessCategory ac = edcaf->getAccessCategory();
        EV_DETAIL << "Channel access granted to the " << printAccessCategory(ac) << " queue" << std::endl;
        edcaTxops[ac]->startTxop(ac);
        auto internallyCollidedEdcafs = edca->getInternallyCollidedEdcafs();
        if (internallyCollidedEdcafs.size() > 0) {
            EV_INFO << "Internal collision happened with the following queues:" << std::endl;
            handleInternalCollision(internallyCollidedEdcafs);
        }
        startFrameSequence(ac);
    }
    else
        throw cRuntimeError("Channel access granted but channel owner not found!");
}

FrameSequenceContext* Hcf::buildContext(AccessCategory ac)
{
    auto qosContext = new QoSContext(originatorAckPolicy, originatorBlockAckProcedure, originatorBlockAckAgreementHandler, edcaTxops[ac]);
    return new FrameSequenceContext(mac->getAddress(), modeSet, edcaInProgressFrames[ac], rtsProcedure, rtsPolicy, nullptr, qosContext);
}

void Hcf::startFrameSequence(AccessCategory ac)
{
    frameSequenceHandler->startFrameSequence(new HcfFs(), buildContext(ac), this);
    updateDisplayString();
}

void Hcf::handleInternalCollision(std::vector<Edcaf*> internallyCollidedEdcafs)
{
    for (auto edcaf : internallyCollidedEdcafs) {
        AccessCategory ac = edcaf->getAccessCategory();
        Packet *internallyCollidedFrame = edcaInProgressFrames[ac]->getFrameToTransmit();
        auto internallyCollidedHeader = internallyCollidedFrame->peekHeader<Ieee80211DataOrMgmtHeader>();
        EV_INFO << printAccessCategory(ac) << " (" << internallyCollidedFrame->getName() << ")" << endl;
        bool retryLimitReached = false;
        if (auto dataFrame = std::dynamic_pointer_cast<Ieee80211DataHeader>(internallyCollidedHeader)) { // TODO: QoSDataFrame
            edcaDataRecoveryProcedures[ac]->dataFrameTransmissionFailed(internallyCollidedFrame, dataFrame);
            retryLimitReached = edcaDataRecoveryProcedures[ac]->isRetryLimitReached(internallyCollidedFrame, dataFrame);
        }
        else if (auto mgmtFrame = std::dynamic_pointer_cast<Ieee80211MgmtHeader>(internallyCollidedHeader)) {
            ASSERT(ac == AccessCategory::AC_BE);
            edcaMgmtAndNonQoSRecoveryProcedure->dataOrMgmtFrameTransmissionFailed(internallyCollidedFrame, mgmtFrame, stationRetryCounters[AccessCategory::AC_BE]);
            retryLimitReached = edcaMgmtAndNonQoSRecoveryProcedure->isRetryLimitReached(internallyCollidedFrame, mgmtFrame);
        }
        else // TODO: + NonQoSDataFrame
            throw cRuntimeError("Unknown frame");
        if (retryLimitReached) {
            EV_DETAIL << "The frame has reached its retry limit. Dropping it" << std::endl;
            if (auto dataFrame = std::dynamic_pointer_cast<Ieee80211DataHeader>(internallyCollidedHeader))
                edcaDataRecoveryProcedures[ac]->retryLimitReached(internallyCollidedFrame, dataFrame);
            else if (auto mgmtFrame = std::dynamic_pointer_cast<Ieee80211MgmtHeader>(internallyCollidedHeader))
                edcaMgmtAndNonQoSRecoveryProcedure->retryLimitReached(internallyCollidedFrame, mgmtFrame);
            else ; // TODO: + NonQoSDataFrame
            emit(NF_PACKET_DROP, internallyCollidedFrame);
            emit(NF_LINK_BREAK, internallyCollidedFrame);
            edcaInProgressFrames[ac]->dropFrame(internallyCollidedFrame);
            if (hasFrameToTransmit(ac))
                edcaf->requestChannel(this);
        }
        else
            edcaf->requestChannel(this);
    }
}

/*
 * TODO:  If a PHY-RXSTART.indication primitive does not occur during the ACKTimeout interval,
 * the STA concludes that the transmission of the MPDU has failed, and this STA shall invoke its
 * backoff procedure **upon expiration of the ACKTimeout interval**.
 */

void Hcf::frameSequenceFinished()
{
    auto edcaf = edca->getChannelOwner();
    if (edcaf) {
        bool startContention = hasFrameToTransmit(); // TODO: outstanding frame
        edcaf->releaseChannel(this);
        mac->sendDownPendingRadioConfigMsg(); // TODO: review
        AccessCategory ac = edcaf->getAccessCategory();
        edcaTxops[ac]->stopTxop();
        if (startContention)
            edcaf->requestChannel(this);
    }
    else if (hcca->isOwning()) {
        hcca->releaseChannel(this);
        mac->sendDownPendingRadioConfigMsg(); // TODO: review
        throw cRuntimeError("Hcca is unimplemented!");
    }
    else
        throw cRuntimeError("Frame sequence finished but channel owner not found!");
}

void Hcf::recipientProcessReceivedFrame(Packet *packet, const Ptr<Ieee80211MacHeader>& frame)
{
    if (auto dataOrMgmtFrame = std::dynamic_pointer_cast<Ieee80211DataOrMgmtHeader>(frame))
        recipientAckProcedure->processReceivedFrame(packet, dataOrMgmtFrame, check_and_cast<IRecipientAckPolicy*>(recipientAckPolicy), this);
    if (auto dataFrame = std::dynamic_pointer_cast<Ieee80211DataHeader>(frame)) {
        if (dataFrame->getType() == ST_DATA_WITH_QOS && recipientBlockAckAgreementHandler)
            recipientBlockAckAgreementHandler->qosFrameReceived(dataFrame, this);
        sendUp(recipientDataService->dataFrameReceived(packet, dataFrame, recipientBlockAckAgreementHandler));
    }
    else if (auto mgmtFrame = std::dynamic_pointer_cast<Ieee80211MgmtHeader>(frame)) {
        sendUp(recipientDataService->managementFrameReceived(packet, mgmtFrame));
        recipientProcessReceivedManagementFrame(mgmtFrame);
    }
    else { // TODO: else if (auto ctrlFrame = dynamic_cast<Ieee80211ControlFrame*>(frame))
        sendUp(recipientDataService->controlFrameReceived(packet, frame, recipientBlockAckAgreementHandler));
        recipientProcessReceivedControlFrame(packet, frame);
    }
}

void Hcf::recipientProcessReceivedControlFrame(Packet *packet, const Ptr<Ieee80211MacHeader>& frame)
{
    if (auto rtsFrame = std::dynamic_pointer_cast<Ieee80211RtsFrame>(frame))
        ctsProcedure->processReceivedRts(packet, rtsFrame, ctsPolicy, this);
    else if (auto blockAckRequest = std::dynamic_pointer_cast<Ieee80211BasicBlockAckReq>(frame)) {
        if (recipientBlockAckProcedure)
            recipientBlockAckProcedure->processReceivedBlockAckReq(packet, blockAckRequest, recipientAckPolicy, recipientBlockAckAgreementHandler, this);
    }
    else if (auto ackFrame = std::dynamic_pointer_cast<Ieee80211AckFrame>(frame))
        ; // drop it, it is an ACK frame that is received after the ACKTimeout
    else
        throw cRuntimeError("Unknown control frame");
}

void Hcf::recipientProcessReceivedManagementFrame(const Ptr<Ieee80211MgmtHeader>& frame)
{
    if (recipientBlockAckAgreementHandler && originatorBlockAckAgreementHandler) {
        if (auto addbaRequest = std::dynamic_pointer_cast<Ieee80211AddbaRequest>(frame))
            recipientBlockAckAgreementHandler->processReceivedAddbaRequest(addbaRequest, recipientBlockAckAgreementPolicy, this);
        else if (auto addbaResp = std::dynamic_pointer_cast<Ieee80211AddbaResponse>(frame))
            originatorBlockAckAgreementHandler->processReceivedAddbaResp(addbaResp, originatorBlockAckAgreementPolicy, this);
        else if (auto delba = std::dynamic_pointer_cast<Ieee80211Delba>(frame)) {
            if (delba->getInitiator())
                recipientBlockAckAgreementHandler->processReceivedDelba(delba, recipientBlockAckAgreementPolicy);
            else
                originatorBlockAckAgreementHandler->processReceivedDelba(delba, originatorBlockAckAgreementPolicy);
        }
        else
            ; // Beacon, etc
    }
    else
        ; // Optional modules
}

void Hcf::transmissionComplete(Packet *packet, const Ptr<Ieee80211MacHeader>& frame)
{
    auto edcaf = edca->getChannelOwner();
    if (edcaf) {
        frameSequenceHandler->transmissionComplete();
        updateDisplayString();
    }
    else if (hcca->isOwning())
        throw cRuntimeError("Hcca is unimplemented!");
    else
        recipientProcessTransmittedControlResponseFrame(frame);
    delete packet;
}

void Hcf::originatorProcessRtsProtectionFailed(Packet *packet)
{
    auto protectedFrame = packet->peekHeader<Ieee80211DataOrMgmtHeader>();
    auto edcaf = edca->getChannelOwner();
    if (edcaf) {
        EV_INFO << "RTS frame transmission failed\n";
        AccessCategory ac = edcaf->getAccessCategory();
        bool retryLimitReached = false;
        if (auto dataFrame = std::dynamic_pointer_cast<Ieee80211DataHeader>(protectedFrame)) {
            edcaDataRecoveryProcedures[ac]->rtsFrameTransmissionFailed(dataFrame);
            retryLimitReached = edcaDataRecoveryProcedures[ac]->isRtsFrameRetryLimitReached(packet, dataFrame);
        }
        else if (auto mgmtFrame = std::dynamic_pointer_cast<Ieee80211MgmtHeader>(protectedFrame)) {
            edcaMgmtAndNonQoSRecoveryProcedure->rtsFrameTransmissionFailed(mgmtFrame, stationRetryCounters[ac]);
            retryLimitReached = edcaMgmtAndNonQoSRecoveryProcedure->isRtsFrameRetryLimitReached(packet, dataFrame);
        }
        else
            throw cRuntimeError("Unknown frame"); // TODO: QoSDataFrame, NonQoSDataFrame
        if (retryLimitReached) {
            if (auto dataFrame = std::dynamic_pointer_cast<Ieee80211DataHeader>(protectedFrame))
                edcaDataRecoveryProcedures[ac]->retryLimitReached(packet, dataFrame);
            else if (auto mgmtFrame = std::dynamic_pointer_cast<Ieee80211MgmtHeader>(protectedFrame))
                edcaMgmtAndNonQoSRecoveryProcedure->retryLimitReached(packet, mgmtFrame);
            else ; // TODO: nonqos data
            edcaInProgressFrames[ac]->dropFrame(packet);
            emit(NF_PACKET_DROP, packet);
            emit(NF_LINK_BREAK, packet);
            delete packet;
        }
    }
    else
        throw cRuntimeError("Hcca is unimplemented!");
}

void Hcf::originatorProcessTransmittedFrame(Packet *packet)
{
    auto transmittedFrame = packet->peekHeader<Ieee80211MacHeader>();
    auto edcaf = edca->getChannelOwner();
    if (edcaf) {
        AccessCategory ac = edcaf->getAccessCategory();
        if (transmittedFrame->getReceiverAddress().isMulticast()) {
            edcaDataRecoveryProcedures[ac]->multicastFrameTransmitted();
            if (auto transmittedDataOrMgmtFrame = std::dynamic_pointer_cast<Ieee80211DataOrMgmtHeader>(transmittedFrame))
                edcaInProgressFrames[ac]->dropFrame(packet);
        }
        else if (auto dataFrame = std::dynamic_pointer_cast<Ieee80211DataHeader>(transmittedFrame))
            originatorProcessTransmittedDataFrame(packet, dataFrame, ac);
        else if (auto mgmtFrame = std::dynamic_pointer_cast<Ieee80211MgmtHeader>(transmittedFrame))
            originatorProcessTransmittedManagementFrame(mgmtFrame, ac);
        else // TODO: Ieee80211ControlFrame
            originatorProcessTransmittedControlFrame(transmittedFrame, ac);
    }
    else if (hcca->isOwning())
        throw cRuntimeError("Hcca is unimplemented");
    else
        throw cRuntimeError("Frame transmitted but channel owner not found");
}

void Hcf::originatorProcessTransmittedDataFrame(Packet *packet, const Ptr<Ieee80211DataHeader>& dataFrame, AccessCategory ac)
{
    edcaAckHandlers[ac]->processTransmittedDataOrMgmtFrame(dataFrame);
    if (originatorBlockAckAgreementHandler)
        originatorBlockAckAgreementHandler->processTransmittedDataFrame(packet, dataFrame, originatorBlockAckAgreementPolicy, this);
    if (dataFrame->getAckPolicy() == NO_ACK)
        edcaInProgressFrames[ac]->dropFrame(packet);
}

void Hcf::originatorProcessTransmittedManagementFrame(const Ptr<Ieee80211MgmtHeader>& mgmtFrame, AccessCategory ac)
{
    if (originatorAckPolicy->isAckNeeded(mgmtFrame))
        edcaAckHandlers[ac]->processTransmittedDataOrMgmtFrame(mgmtFrame);
    if (auto addbaReq = std::dynamic_pointer_cast<Ieee80211AddbaRequest>(mgmtFrame)) {
        if (originatorBlockAckAgreementHandler)
            originatorBlockAckAgreementHandler->processTransmittedAddbaReq(addbaReq);
    }
    else if (auto addbaResp = std::dynamic_pointer_cast<Ieee80211AddbaResponse>(mgmtFrame))
        recipientBlockAckAgreementHandler->processTransmittedAddbaResp(addbaResp, this);
    else if (auto delba = std::dynamic_pointer_cast<Ieee80211Delba>(mgmtFrame)) {
        if (delba->getInitiator())
            originatorBlockAckAgreementHandler->processTransmittedDelba(delba);
        else
            recipientBlockAckAgreementHandler->processTransmittedDelba(delba);
    }
    else ; // TODO: other mgmt frames if needed
}

void Hcf::originatorProcessTransmittedControlFrame(const Ptr<Ieee80211MacHeader>& controlFrame, AccessCategory ac)
{
    if (auto blockAckReq = std::dynamic_pointer_cast<Ieee80211BlockAckReq>(controlFrame))
        edcaAckHandlers[ac]->processTransmittedBlockAckReq(blockAckReq);
    else if (auto rtsFrame = std::dynamic_pointer_cast<Ieee80211RtsFrame>(controlFrame))
        rtsProcedure->processTransmittedRts(rtsFrame);
    else
        throw cRuntimeError("Unknown control frame");
}

void Hcf::originatorProcessFailedFrame(Packet *packet)
{
    auto failedFrame = packet->peekHeader<Ieee80211DataOrMgmtHeader>();
    auto edcaf = edca->getChannelOwner();
    if (edcaf) {
        AccessCategory ac = edcaf->getAccessCategory();
        bool retryLimitReached = false;
        if (auto dataFrame = std::dynamic_pointer_cast<Ieee80211DataHeader>(failedFrame)) {
            EV_INFO << "Data frame transmission failed\n";
            if (dataFrame->getAckPolicy() == NORMAL_ACK) {
                edcaDataRecoveryProcedures[ac]->dataFrameTransmissionFailed(packet, dataFrame);
                retryLimitReached = edcaDataRecoveryProcedures[ac]->isRetryLimitReached(packet, dataFrame);
                if (dataAndMgmtRateControl) {
                    int retryCount = edcaDataRecoveryProcedures[ac]->getRetryCount(packet, dataFrame);
                    dataAndMgmtRateControl->frameTransmitted(packet, retryCount, false, retryLimitReached);
                }
            }
            else if (dataFrame->getAckPolicy() == BLOCK_ACK) {
                // TODO:
                // bool lifetimeExpired = lifetimeHandler->isLifetimeExpired(failedFrame);
                // if (lifetimeExpired) {
                //    inProgressFrames->dropFrame(failedFrame);
                //    delete dataFrame;
                // }
            }
            else
                throw cRuntimeError("Unimplemented!");
        }
        else if (auto mgmtFrame = std::dynamic_pointer_cast<Ieee80211MgmtHeader>(failedFrame)) { // TODO: + NonQoS frames
            EV_INFO << "Management frame transmission failed\n";
            edcaMgmtAndNonQoSRecoveryProcedure->dataOrMgmtFrameTransmissionFailed(packet, mgmtFrame, stationRetryCounters[ac]);
            retryLimitReached = edcaMgmtAndNonQoSRecoveryProcedure->isRetryLimitReached(packet, mgmtFrame);
            if (dataAndMgmtRateControl) {
                int retryCount = edcaMgmtAndNonQoSRecoveryProcedure->getRetryCount(packet, mgmtFrame);
                dataAndMgmtRateControl->frameTransmitted(packet, retryCount, false, retryLimitReached);
            }
        }
        else
            throw cRuntimeError("Unknown frame"); // TODO: qos, nonqos
        edcaAckHandlers[ac]->processFailedFrame(failedFrame);
        if (retryLimitReached) {
            if (auto dataFrame = std::dynamic_pointer_cast<Ieee80211DataHeader>(failedFrame))
                edcaDataRecoveryProcedures[ac]->retryLimitReached(packet, dataFrame);
            else if (auto mgmtFrame = std::dynamic_pointer_cast<Ieee80211MgmtHeader>(failedFrame))
                edcaMgmtAndNonQoSRecoveryProcedure->retryLimitReached(packet, mgmtFrame);
            edcaInProgressFrames[ac]->dropFrame(packet);
            // KLUDGE: removed headers and trailers to allow higher layer protocols to process the packet
            packet->popHeader<Ieee80211DataOrMgmtHeader>();
            const auto& nextHeader = packet->peekHeader();
            if (std::dynamic_pointer_cast<Ieee8022LlcHeader>(nextHeader))
                packet->popHeader<Ieee8022LlcHeader>();
            packet->popTrailer<Ieee80211MacTrailer>();
            emit(NF_PACKET_DROP, packet);
            emit(NF_LINK_BREAK, packet);
            delete packet;
        }
        else {
            auto h = packet->removeHeader<Ieee80211DataOrMgmtHeader>();
            h->setRetry(true);
            packet->insertHeader(h);
        }
    }
    else
        throw cRuntimeError("Hcca is unimplemented!");
}

void Hcf::originatorProcessReceivedFrame(Packet *packet, Packet *lastTransmittedPacket)
{
    auto frame = packet->peekHeader<Ieee80211MacHeader>();
    auto lastTransmittedFrame = lastTransmittedPacket->peekHeader<Ieee80211MacHeader>();
    auto edcaf = edca->getChannelOwner();
    if (edcaf) {
        AccessCategory ac = edcaf->getAccessCategory();
        if (auto dataFrame = std::dynamic_pointer_cast<Ieee80211DataHeader>(frame))
            originatorProcessReceivedDataFrame(dataFrame, lastTransmittedFrame, ac);
        else if (auto mgmtFrame = std::dynamic_pointer_cast<Ieee80211MgmtHeader>(frame))
            originatorProcessReceivedManagementFrame(mgmtFrame, lastTransmittedFrame, ac);
        else
            originatorProcessReceivedControlFrame(packet, frame, lastTransmittedPacket, lastTransmittedFrame, ac);
    }
    else
        throw cRuntimeError("Hcca is unimplemented!");
    delete packet;
}

void Hcf::originatorProcessReceivedManagementFrame(const Ptr<Ieee80211MgmtHeader>& frame, const Ptr<Ieee80211MacHeader>& lastTransmittedFrame, AccessCategory ac)
{
    throw cRuntimeError("Unknown management frame");
}

void Hcf::originatorProcessReceivedControlFrame(Packet *packet, const Ptr<Ieee80211MacHeader>& frame, Packet *lastTransmittedPacket, const Ptr<Ieee80211MacHeader>& lastTransmittedFrame, AccessCategory ac)
{
    if (auto ackFrame = std::dynamic_pointer_cast<Ieee80211AckFrame>(frame)) {
        if (auto dataFrame = std::dynamic_pointer_cast<Ieee80211DataHeader>(lastTransmittedFrame)) {
            if (dataAndMgmtRateControl) {
                int retryCount;
                if (dataFrame->getRetry())
                    retryCount = edcaDataRecoveryProcedures[ac]->getRetryCount(packet, dataFrame);
                else
                    retryCount = 0;
                edcaDataRecoveryProcedures[ac]->ackFrameReceived(packet, dataFrame);
                dataAndMgmtRateControl->frameTransmitted(packet, retryCount, true, false);
            }
        }
        else if (auto mgmtFrame = std::dynamic_pointer_cast<Ieee80211MgmtHeader>(lastTransmittedFrame)) {
            if (dataAndMgmtRateControl) {
                int retryCount = edcaMgmtAndNonQoSRecoveryProcedure->getRetryCount(packet, dataFrame);
                dataAndMgmtRateControl->frameTransmitted(packet, retryCount, true, false);
            }
            edcaMgmtAndNonQoSRecoveryProcedure->ackFrameReceived(packet, mgmtFrame, stationRetryCounters[ac]);
        }
        else
            throw cRuntimeError("Unknown frame"); // TODO: qos, nonqos frame
        edcaAckHandlers[ac]->processReceivedAck(ackFrame, std::dynamic_pointer_cast<Ieee80211DataOrMgmtHeader>(lastTransmittedFrame));
        edcaInProgressFrames[ac]->dropFrame(lastTransmittedPacket);
    }
    else if (auto blockAck = std::dynamic_pointer_cast<Ieee80211BasicBlockAck>(frame)) {
        EV_INFO << "BasicBlockAck has arrived" << std::endl;
        edcaDataRecoveryProcedures[ac]->blockAckFrameReceived();
        auto ackedSeqAndFragNums = edcaAckHandlers[ac]->processReceivedBlockAck(blockAck);
        if (originatorBlockAckAgreementHandler)
            originatorBlockAckAgreementHandler->processReceivedBlockAck(blockAck, this);
        EV_INFO << "It has acknowledged the following frames:" << std::endl;
        // FIXME
//        for (auto seqCtrlField : ackedSeqAndFragNums)
//            EV_INFO << "Fragment number = " << seqCtrlField.getSequenceNumber() << " Sequence number = " << (int)seqCtrlField.getFragmentNumber() << std::endl;
        edcaInProgressFrames[ac]->dropFrames(ackedSeqAndFragNums);
    }
    else if (std::dynamic_pointer_cast<Ieee80211RtsFrame>(frame))
        ; // void
    else if (std::dynamic_pointer_cast<Ieee80211CtsFrame>(frame))
        edcaDataRecoveryProcedures[ac]->ctsFrameReceived();
    else if (frame->getType() == ST_DATA_WITH_QOS)
        ; // void
    else if (std::dynamic_pointer_cast<Ieee80211BasicBlockAckReq>(frame))
        ; // void
    else
        throw cRuntimeError("Unknown control frame");
}

void Hcf::originatorProcessReceivedDataFrame(const Ptr<Ieee80211DataHeader>& frame, const Ptr<Ieee80211MacHeader>& lastTransmittedFrame, AccessCategory ac)
{
    throw cRuntimeError("Unknown data frame");
}

bool Hcf::hasFrameToTransmit(AccessCategory ac)
{
    auto edcaf = edca->getChannelOwner();
    if (edcaf) {
        AccessCategory ac = edcaf->getAccessCategory();
        return !edcaPendingQueues[ac]->isEmpty() || edcaInProgressFrames[ac]->hasInProgressFrames();
    }
    else
        throw cRuntimeError("Hcca is unimplemented");
}

bool Hcf::hasFrameToTransmit()
{
    auto edcaf = edca->getChannelOwner();
    if (edcaf) {
        AccessCategory ac = edcaf->getAccessCategory();
        return !edcaPendingQueues[ac]->isEmpty() || edcaInProgressFrames[ac]->hasInProgressFrames();
    }
    else
        throw cRuntimeError("Hcca is unimplemented");
}

void Hcf::sendUp(const std::vector<Packet*>& completeFrames)
{
    for (auto frame : completeFrames) {
        // FIXME: mgmt module does not handle addba req ..
        const auto& header = frame->peekHeader<Ieee80211DataOrMgmtHeader>();
        if (!std::dynamic_pointer_cast<Ieee80211AddbaRequest>(header) && !std::dynamic_pointer_cast<Ieee80211AddbaResponse>(header) && !std::dynamic_pointer_cast<Ieee80211Delba>(header))
            mac->sendUpFrame(frame);
    }
}

void Hcf::transmitFrame(Packet *packet, simtime_t ifs)
{
    auto channelOwner = edca->getChannelOwner();
    if (channelOwner) {
        auto header = packet->peekHeader<Ieee80211MacHeader>();
        AccessCategory ac = channelOwner->getAccessCategory();
        auto txop = edcaTxops[ac];
        if (auto dataFrame = std::dynamic_pointer_cast<Ieee80211DataHeader>(header)) {
            OriginatorBlockAckAgreement *agreement = nullptr;
            if (originatorBlockAckAgreementHandler)
                agreement = originatorBlockAckAgreementHandler->getAgreement(dataFrame->getReceiverAddress(), dataFrame->getTid());
            auto ackPolicy = originatorAckPolicy->computeAckPolicy(packet, dataFrame, agreement);
            auto dataHeader = packet->removeHeader<Ieee80211DataHeader>();
            dataHeader->setAckPolicy(ackPolicy);
            packet->insertHeader(dataHeader);
        }
        setFrameMode(packet, header, rateSelection->computeMode(packet, header, txop));
        if (txop->getProtectionMechanism() == TxopProcedure::ProtectionMechanism::SINGLE_PROTECTION) {
            auto pendingPacket = edcaInProgressFrames[ac]->getPendingFrameFor(packet);
            const auto& pendingFrame = pendingPacket == nullptr ? nullptr : pendingPacket->peekHeader<Ieee80211DataOrMgmtHeader>();
            auto duration = singleProtectionMechanism->computeDurationField(packet, header, pendingPacket, pendingFrame, edcaTxops[ac], recipientAckPolicy);
            auto header = packet->removeHeader<Ieee80211MacHeader>();
            header->setDuration(duration);
            packet->insertHeader(header);
        }
        else if (txop->getProtectionMechanism() == TxopProcedure::ProtectionMechanism::MULTIPLE_PROTECTION)
            throw cRuntimeError("Multiple protection is unsupported");
        else
            throw cRuntimeError("Undefined protection mechanism");
        tx->transmitFrame(packet, packet->peekHeader<Ieee80211MacHeader>(), ifs, this);
    }
    else
        throw cRuntimeError("Hcca is unimplemented");
}

void Hcf::transmitControlResponseFrame(Packet *responsePacket, const Ptr<Ieee80211MacHeader>& responseFrame, Packet *receivedPacket, const Ptr<Ieee80211MacHeader>& receivedFrame)
{
    responsePacket->insertTrailer(std::make_shared<Ieee80211MacTrailer>());
    const IIeee80211Mode *responseMode = nullptr;
    if (auto rtsFrame = std::dynamic_pointer_cast<Ieee80211RtsFrame>(receivedFrame))
        responseMode = rateSelection->computeResponseCtsFrameMode(receivedPacket, rtsFrame);
    else if (auto blockAckReq = std::dynamic_pointer_cast<Ieee80211BasicBlockAckReq>(receivedFrame))
        responseMode = rateSelection->computeResponseBlockAckFrameMode(receivedPacket, blockAckReq);
    else if (auto dataOrMgmtFrame = std::dynamic_pointer_cast<Ieee80211DataOrMgmtHeader>(receivedFrame))
        responseMode = rateSelection->computeResponseAckFrameMode(receivedPacket, dataOrMgmtFrame);
    else
        throw cRuntimeError("Unknown received frame type");
    setFrameMode(responsePacket, responseFrame, responseMode);
    tx->transmitFrame(responsePacket, responseFrame, modeSet->getSifsTime(), this);
    delete responsePacket;
}

void Hcf::recipientProcessTransmittedControlResponseFrame(const Ptr<Ieee80211MacHeader>& frame)
{
    if (auto ctsFrame = std::dynamic_pointer_cast<Ieee80211CtsFrame>(frame))
        ctsProcedure->processTransmittedCts(ctsFrame);
    else if (auto blockAck = std::dynamic_pointer_cast<Ieee80211BlockAck>(frame)) {
        if (recipientBlockAckProcedure)
            recipientBlockAckProcedure->processTransmittedBlockAck(blockAck);
    }
    else if (auto ackFrame = std::dynamic_pointer_cast<Ieee80211AckFrame>(frame))
        recipientAckProcedure->processTransmittedAck(ackFrame);
    else
        throw cRuntimeError("Unknown control response frame");
}


void Hcf::processMgmtFrame(Packet *mgmtPacket, const Ptr<Ieee80211MgmtHeader>& mgmtFrame)
{
    mgmtPacket->insertTrailer(std::make_shared<Ieee80211MacTrailer>());
    processUpperFrame(mgmtPacket, mgmtFrame);
}

void Hcf::setFrameMode(Packet *packet, const Ptr<Ieee80211MacHeader>& frame, const IIeee80211Mode *mode) const
 {
    ASSERT(mode != nullptr);
    packet->ensureTag<Ieee80211ModeReq>()->setMode(mode);
}


bool Hcf::isReceptionInProgress()
{
    return rx->isReceptionInProgress();
}

bool Hcf::isForUs(const Ptr<Ieee80211MacHeader>& frame) const
{
    return frame->getReceiverAddress() == mac->getAddress() || (frame->getReceiverAddress().isMulticast() && !isSentByUs(frame));
}

bool Hcf::isSentByUs(const Ptr<Ieee80211MacHeader>& frame) const
{
    // FIXME:
    // Check the roles of the Addr3 field when aggregation is applied
    // Table 8-19—Address field contents
    if (auto dataOrMgmtFrame = std::dynamic_pointer_cast<Ieee80211DataOrMgmtHeader>(frame))
        return dataOrMgmtFrame->getAddress3() == mac->getAddress();
    else
        return false;
}

void Hcf::corruptedFrameReceived()
{
    if (frameSequenceHandler->isSequenceRunning()) {
        if (!startRxTimer->isScheduled()) {
            frameSequenceHandler->handleStartRxTimeout();
            updateDisplayString();
        }
    }
}

Hcf::~Hcf()
{
    cancelAndDelete(startRxTimer);
    cancelAndDelete(inactivityTimer);
    delete recipientAckProcedure;
    delete ctsProcedure;
    delete rtsProcedure;
    delete originatorBlockAckAgreementHandler;
    delete recipientBlockAckAgreementHandler;
    delete originatorBlockAckProcedure;
    delete recipientBlockAckProcedure;
    delete frameSequenceHandler;
    for (auto inProgressFrames : edcaInProgressFrames)
        delete inProgressFrames;
    for (auto pendingQueue : edcaPendingQueues)
        delete pendingQueue;
    for (auto ackHandler : edcaAckHandlers)
        delete ackHandler;
    for (auto retryCounter : stationRetryCounters)
        delete retryCounter;
}

} // namespace ieee80211
} // namespace inet
