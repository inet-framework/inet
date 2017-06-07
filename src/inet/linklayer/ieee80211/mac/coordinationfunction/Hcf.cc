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
#include "inet/linklayer/ieee80211/mac/blockack/OriginatorBlockAckAgreementHandler.h"
#include "inet/linklayer/ieee80211/mac/blockack/OriginatorBlockAckProcedure.h"
#include "inet/linklayer/ieee80211/mac/blockack/RecipientBlockAckAgreementHandler.h"
#include "inet/linklayer/ieee80211/mac/coordinationfunction/Hcf.h"
#include "inet/linklayer/ieee80211/mac/framesequence/HcfFs.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Mac.h"
#include "inet/linklayer/ieee80211/mac/recipient/RecipientAckProcedure.h"

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
        if (!isReceptionInProgress())
            frameSequenceHandler->handleStartRxTimeout();
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

void Hcf::processUpperFrame(Ieee80211DataOrMgmtFrame* frame)
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
    if (dynamic_cast<Ieee80211ManagementFrame *>(frame)) // TODO: + non-QoS frames
        ac = AccessCategory::AC_VO;
    else if (auto dataFrame = dynamic_cast<Ieee80211DataFrame *>(frame))
        ac = edca->classifyFrame(dataFrame);
    else
        throw cRuntimeError("Unknown message type");
    EV_INFO << "The upper frame has been classified as a " << printAccessCategory(ac) << " frame." << endl;
    if (edcaPendingQueues[ac]->insert(frame)) {
        EV_INFO << "Frame " << frame->getName() << " has been inserted into the PendingQueue." << endl;
        auto edcaf = edca->getChannelOwner();
        if (edcaf == nullptr || edcaf->getAccessCategory() != ac) {
            EV_DETAIL << "Requesting channel for access category " << printAccessCategory(ac) << endl;
            edca->requestChannelAccess(ac, this);
        }
    }
    else {
        EV_INFO << "Frame " << frame->getName() << " has been dropped because the PendingQueue is full." << endl;
        emit(NF_PACKET_DROP, frame);
        delete frame;
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

void Hcf::processLowerFrame(Ieee80211Frame* frame)
{
    Enter_Method_Silent();
    auto edcaf = edca->getChannelOwner();
    if (edcaf && frameSequenceHandler->isSequenceRunning()) {
        // TODO: always call processResponse?
        if ((!isForUs(frame) && !startRxTimer->isScheduled()) || isForUs(frame))
            frameSequenceHandler->processResponse(frame);
        else {
            EV_INFO << "This frame is not for us" << std::endl;
            delete frame;
        }
        cancelEvent(startRxTimer);
    }
    else if (hcca->isOwning())
        throw cRuntimeError("Hcca is unimplemented!");
    else if (isForUs(frame))
        recipientProcessReceivedFrame(frame);
    else {
        EV_INFO << "This frame is not for us" << std::endl;
        delete frame;
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
}

void Hcf::handleInternalCollision(std::vector<Edcaf*> internallyCollidedEdcafs)
{
    for (auto edcaf : internallyCollidedEdcafs) {
        AccessCategory ac = edcaf->getAccessCategory();
        Ieee80211DataOrMgmtFrame *internallyCollidedFrame = edcaInProgressFrames[ac]->getFrameToTransmit();
        EV_INFO << printAccessCategory(ac) << " (" << internallyCollidedFrame->getName() << ")" << endl;
        bool retryLimitReached = false;
        if (auto dataFrame = dynamic_cast<Ieee80211DataFrame *>(internallyCollidedFrame)) { // TODO: QoSDataFrame
            edcaDataRecoveryProcedures[ac]->dataFrameTransmissionFailed(dataFrame);
            retryLimitReached = edcaDataRecoveryProcedures[ac]->isRetryLimitReached(dataFrame);
        }
        else if (auto mgmtFrame = dynamic_cast<Ieee80211ManagementFrame*>(internallyCollidedFrame)) {
            ASSERT(ac == AccessCategory::AC_BE);
            edcaMgmtAndNonQoSRecoveryProcedure->dataOrMgmtFrameTransmissionFailed(mgmtFrame, stationRetryCounters[AccessCategory::AC_BE]);
            retryLimitReached = edcaMgmtAndNonQoSRecoveryProcedure->isRetryLimitReached(mgmtFrame);
        }
        else // TODO: + NonQoSDataFrame
            throw cRuntimeError("Unknown frame");
        if (retryLimitReached) {
            EV_DETAIL << "The frame has reached its retry limit. Dropping it" << std::endl;
            emit(NF_LINK_BREAK, internallyCollidedFrame);
            emit(NF_PACKET_DROP, internallyCollidedFrame);
            if (auto dataFrame = dynamic_cast<Ieee80211DataFrame *>(internallyCollidedFrame))
                edcaDataRecoveryProcedures[ac]->retryLimitReached(dataFrame);
            else if (auto mgmtFrame = dynamic_cast<Ieee80211ManagementFrame*>(internallyCollidedFrame))
                edcaMgmtAndNonQoSRecoveryProcedure->retryLimitReached(mgmtFrame);
            else ; // TODO: + NonQoSDataFrame
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

void Hcf::recipientProcessReceivedFrame(Ieee80211Frame* frame)
{
    if (auto dataOrMgmtFrame = dynamic_cast<Ieee80211DataOrMgmtFrame *>(frame))
        recipientAckProcedure->processReceivedFrame(dataOrMgmtFrame, check_and_cast<IRecipientAckPolicy*>(recipientAckPolicy), this);
    if (auto dataFrame = dynamic_cast<Ieee80211DataFrame*>(frame)) {
        if (dataFrame->getType() == ST_DATA_WITH_QOS && recipientBlockAckAgreementHandler)
            recipientBlockAckAgreementHandler->qosFrameReceived(dataFrame, this);
        sendUp(recipientDataService->dataFrameReceived(dataFrame, recipientBlockAckAgreementHandler));
    }
    else if (auto mgmtFrame = dynamic_cast<Ieee80211ManagementFrame*>(frame)) {
        sendUp(recipientDataService->managementFrameReceived(mgmtFrame));
        recipientProcessReceivedManagementFrame(mgmtFrame);
        if (dynamic_cast<Ieee80211ActionFrame *>(mgmtFrame))
            delete mgmtFrame;
    }
    else { // TODO: else if (auto ctrlFrame = dynamic_cast<Ieee80211ControlFrame*>(frame))
        sendUp(recipientDataService->controlFrameReceived(frame, recipientBlockAckAgreementHandler));
        recipientProcessReceivedControlFrame(frame);
        delete frame;
    }
}

void Hcf::recipientProcessReceivedControlFrame(Ieee80211Frame* frame)
{
    if (auto rtsFrame = dynamic_cast<Ieee80211RTSFrame *>(frame))
        ctsProcedure->processReceivedRts(rtsFrame, ctsPolicy, this);
    else if (auto blockAckRequest = dynamic_cast<Ieee80211BasicBlockAckReq*>(frame)) {
        if (recipientBlockAckProcedure)
            recipientBlockAckProcedure->processReceivedBlockAckReq(blockAckRequest, recipientAckPolicy, recipientBlockAckAgreementHandler, this);
    }
    else if (dynamic_cast<Ieee80211ACKFrame*>(frame))
        EV_WARN << "ACK frame received after timeout, ignoring it.\n"; // drop it, it is an ACK frame that is received after the ACKTimeout
    else
        throw cRuntimeError("Unknown control frame");
}

void Hcf::recipientProcessReceivedManagementFrame(Ieee80211ManagementFrame* frame)
{
    if (recipientBlockAckAgreementHandler && originatorBlockAckAgreementHandler) {
        if (auto addbaRequest = dynamic_cast<Ieee80211AddbaRequest *>(frame))
            recipientBlockAckAgreementHandler->processReceivedAddbaRequest(addbaRequest, recipientBlockAckAgreementPolicy, this);
        else if (auto addbaResp = dynamic_cast<Ieee80211AddbaResponse *>(frame))
            originatorBlockAckAgreementHandler->processReceivedAddbaResp(addbaResp, originatorBlockAckAgreementPolicy, this);
        else if (auto delba = dynamic_cast<Ieee80211Delba*>(frame)) {
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

void Hcf::transmissionComplete(Ieee80211Frame *frame)
{
    auto edcaf = edca->getChannelOwner();
    if (edcaf)
        frameSequenceHandler->transmissionComplete();
    else if (hcca->isOwning())
        throw cRuntimeError("Hcca is unimplemented!");
    else
        recipientProcessTransmittedControlResponseFrame(frame);
    delete frame;
}

void Hcf::originatorProcessRtsProtectionFailed(Ieee80211DataOrMgmtFrame* protectedFrame)
{
    auto edcaf = edca->getChannelOwner();
    if (edcaf) {
        EV_INFO << "RTS frame transmission failed\n";
        AccessCategory ac = edcaf->getAccessCategory();
        bool retryLimitReached = false;
        if (auto dataFrame = dynamic_cast<Ieee80211DataFrame *>(protectedFrame)) {
            edcaDataRecoveryProcedures[ac]->rtsFrameTransmissionFailed(dataFrame);
            retryLimitReached = edcaDataRecoveryProcedures[ac]->isRtsFrameRetryLimitReached(dataFrame);
        }
        else if (auto mgmtFrame = dynamic_cast<Ieee80211ManagementFrame *>(protectedFrame)) {
            edcaMgmtAndNonQoSRecoveryProcedure->rtsFrameTransmissionFailed(mgmtFrame, stationRetryCounters[ac]);
            retryLimitReached = edcaMgmtAndNonQoSRecoveryProcedure->isRtsFrameRetryLimitReached(dataFrame);
        }
        else
            throw cRuntimeError("Unknown frame"); // TODO: QoSDataFrame, NonQoSDataFrame
        if (retryLimitReached) {
            if (auto dataFrame = dynamic_cast<Ieee80211DataFrame *>(protectedFrame))
                edcaDataRecoveryProcedures[ac]->retryLimitReached(dataFrame);
            else if (auto mgmtFrame = dynamic_cast<Ieee80211ManagementFrame*>(protectedFrame))
                edcaMgmtAndNonQoSRecoveryProcedure->retryLimitReached(mgmtFrame);
            else ; // TODO: nonqos data
            edcaInProgressFrames[ac]->dropFrame(protectedFrame);
            emit(NF_LINK_BREAK, protectedFrame);
            emit(NF_PACKET_DROP, protectedFrame);
        }
    }
    else
        throw cRuntimeError("Hcca is unimplemented!");
}

void Hcf::originatorProcessTransmittedFrame(Ieee80211Frame* transmittedFrame)
{
    auto edcaf = edca->getChannelOwner();
    if (edcaf) {
        AccessCategory ac = edcaf->getAccessCategory();
        if (transmittedFrame->getReceiverAddress().isMulticast()) {
            edcaDataRecoveryProcedures[ac]->multicastFrameTransmitted();
            if (auto transmittedDataOrMgmtFrame = dynamic_cast<Ieee80211DataOrMgmtFrame*>(transmittedFrame))
                edcaInProgressFrames[ac]->dropFrame(transmittedDataOrMgmtFrame);
        }
        else if (auto dataFrame = dynamic_cast<Ieee80211DataFrame*>(transmittedFrame))
            originatorProcessTransmittedDataFrame(dataFrame, ac);
        else if (auto mgmtFrame = dynamic_cast<Ieee80211ManagementFrame*>(transmittedFrame))
            originatorProcessTransmittedManagementFrame(mgmtFrame, ac);
        else // TODO: Ieee80211ControlFrame
            originatorProcessTransmittedControlFrame(transmittedFrame, ac);
    }
    else if (hcca->isOwning())
        throw cRuntimeError("Hcca is unimplemented");
    else
        throw cRuntimeError("Frame transmitted but channel owner not found");
}

void Hcf::originatorProcessTransmittedDataFrame(Ieee80211DataFrame* dataFrame, AccessCategory ac)
{
    edcaAckHandlers[ac]->processTransmittedDataOrMgmtFrame(dataFrame);
    if (originatorBlockAckAgreementHandler)
        originatorBlockAckAgreementHandler->processTransmittedDataFrame(dataFrame, originatorBlockAckAgreementPolicy, this);
    if (dataFrame->getAckPolicy() == NO_ACK)
        edcaInProgressFrames[ac]->dropFrame(dataFrame);
}

void Hcf::originatorProcessTransmittedManagementFrame(Ieee80211ManagementFrame* mgmtFrame, AccessCategory ac)
{
    if (originatorAckPolicy->isAckNeeded(mgmtFrame))
        edcaAckHandlers[ac]->processTransmittedDataOrMgmtFrame(mgmtFrame);
    if (auto addbaReq = dynamic_cast<Ieee80211AddbaRequest*>(mgmtFrame)) {
        if (originatorBlockAckAgreementHandler)
            originatorBlockAckAgreementHandler->processTransmittedAddbaReq(addbaReq);
    }
    else if (auto addbaResp = dynamic_cast<Ieee80211AddbaResponse*>(mgmtFrame))
        recipientBlockAckAgreementHandler->processTransmittedAddbaResp(addbaResp, this);
    else if (auto delba = dynamic_cast<Ieee80211Delba *>(mgmtFrame)) {
        if (delba->getInitiator())
            originatorBlockAckAgreementHandler->processTransmittedDelba(delba);
        else
            recipientBlockAckAgreementHandler->processTransmittedDelba(delba);
    }
    else ; // TODO: other mgmt frames if needed
}

void Hcf::originatorProcessTransmittedControlFrame(Ieee80211Frame* controlFrame, AccessCategory ac)
{
    if (auto blockAckReq = dynamic_cast<Ieee80211BlockAckReq*>(controlFrame))
        edcaAckHandlers[ac]->processTransmittedBlockAckReq(blockAckReq);
    else if (auto rtsFrame = dynamic_cast<Ieee80211RTSFrame*>(controlFrame))
        rtsProcedure->processTransmittedRts(rtsFrame);
    else
        throw cRuntimeError("Unknown control frame");
}

void Hcf::originatorProcessFailedFrame(Ieee80211DataOrMgmtFrame* failedFrame)
{
    auto edcaf = edca->getChannelOwner();
    if (edcaf) {
        AccessCategory ac = edcaf->getAccessCategory();
        bool retryLimitReached = false;
        if (auto dataFrame = dynamic_cast<Ieee80211DataFrame *>(failedFrame)) {
            EV_INFO << "Data frame transmission failed\n";
            if (dataFrame->getAckPolicy() == NORMAL_ACK) {
                edcaDataRecoveryProcedures[ac]->dataFrameTransmissionFailed(dataFrame);
                retryLimitReached = edcaDataRecoveryProcedures[ac]->isRetryLimitReached(dataFrame);
                if (dataAndMgmtRateControl) {
                    int retryCount = edcaDataRecoveryProcedures[ac]->getRetryCount(dataFrame);
                    dataAndMgmtRateControl->frameTransmitted(dataFrame, retryCount, false, retryLimitReached);
                }
            }
            else if (dataFrame->getAckPolicy() == BLOCK_ACK) {
                // TODO:
                // bool lifetimeExpired = lifetimeHandler->isLifetimeExpired(failedFrame);
                // if (lifetimeExpired) {
                //    inProgressFrames->dropFrame(failedFrame);
                // }
            }
            else
                throw cRuntimeError("Unimplemented!");
        }
        else if (auto mgmtFrame = dynamic_cast<Ieee80211ManagementFrame*>(failedFrame)) { // TODO: + NonQoS frames
            EV_INFO << "Management frame transmission failed\n";
            edcaMgmtAndNonQoSRecoveryProcedure->dataOrMgmtFrameTransmissionFailed(mgmtFrame, stationRetryCounters[ac]);
            retryLimitReached = edcaMgmtAndNonQoSRecoveryProcedure->isRetryLimitReached(mgmtFrame);
            if (dataAndMgmtRateControl) {
                int retryCount = edcaMgmtAndNonQoSRecoveryProcedure->getRetryCount(mgmtFrame);
                dataAndMgmtRateControl->frameTransmitted(mgmtFrame, retryCount, false, retryLimitReached);
            }
        }
        else
            throw cRuntimeError("Unknown frame"); // TODO: qos, nonqos
        edcaAckHandlers[ac]->processFailedFrame(failedFrame);
        if (retryLimitReached) {
            if (auto dataFrame = dynamic_cast<Ieee80211DataFrame *>(failedFrame))
                edcaDataRecoveryProcedures[ac]->retryLimitReached(dataFrame);
            else if (auto mgmtFrame = dynamic_cast<Ieee80211ManagementFrame*>(failedFrame))
                edcaMgmtAndNonQoSRecoveryProcedure->retryLimitReached(mgmtFrame);
            edcaInProgressFrames[ac]->dropFrame(failedFrame);
            emit(NF_LINK_BREAK, failedFrame);
            emit(NF_PACKET_DROP, failedFrame);
        }
        else
            failedFrame->setRetry(true);
    }
    else
        throw cRuntimeError("Hcca is unimplemented!");
}

void Hcf::originatorProcessReceivedFrame(Ieee80211Frame* frame, Ieee80211Frame* lastTransmittedFrame)
{
    auto edcaf = edca->getChannelOwner();
    if (edcaf) {
        AccessCategory ac = edcaf->getAccessCategory();
        if (auto dataFrame = dynamic_cast<Ieee80211DataFrame*>(frame))
            originatorProcessReceivedDataFrame(dataFrame, lastTransmittedFrame, ac);
        else if (auto mgmtFrame = dynamic_cast<Ieee80211ManagementFrame *>(frame))
            originatorProcessReceivedManagementFrame(mgmtFrame, lastTransmittedFrame, ac);
        else
            originatorProcessReceivedControlFrame(frame, lastTransmittedFrame, ac);
    }
    else
        throw cRuntimeError("Hcca is unimplemented!");
}

void Hcf::originatorProcessReceivedManagementFrame(Ieee80211ManagementFrame* frame, Ieee80211Frame* lastTransmittedFrame, AccessCategory ac)
{
    throw cRuntimeError("Unknown management frame");
}

void Hcf::originatorProcessReceivedControlFrame(Ieee80211Frame* frame, Ieee80211Frame* lastTransmittedFrame, AccessCategory ac)
{
    if (auto ackFrame = dynamic_cast<Ieee80211ACKFrame *>(frame)) {
        if (auto dataFrame = dynamic_cast<Ieee80211DataFrame *>(lastTransmittedFrame)) {
            if (dataAndMgmtRateControl) {
                int retryCount;
                if (dataFrame->getRetry())
                    retryCount = edcaDataRecoveryProcedures[ac]->getRetryCount(dataFrame);
                else
                    retryCount = 0;
                edcaDataRecoveryProcedures[ac]->ackFrameReceived(dataFrame);
                dataAndMgmtRateControl->frameTransmitted(dataFrame, retryCount, true, false);
            }
        }
        else if (auto mgmtFrame = dynamic_cast<Ieee80211ManagementFrame *>(lastTransmittedFrame)) {
            if (dataAndMgmtRateControl) {
                int retryCount = edcaMgmtAndNonQoSRecoveryProcedure->getRetryCount(dataFrame);
                dataAndMgmtRateControl->frameTransmitted(mgmtFrame, retryCount, true, false);
            }
            edcaMgmtAndNonQoSRecoveryProcedure->ackFrameReceived(mgmtFrame, stationRetryCounters[ac]);
        }
        else
            throw cRuntimeError("Unknown frame"); // TODO: qos, nonqos frame
        edcaAckHandlers[ac]->processReceivedAck(ackFrame, check_and_cast<Ieee80211DataOrMgmtFrame*>(lastTransmittedFrame));
        edcaInProgressFrames[ac]->dropFrame(check_and_cast<Ieee80211DataOrMgmtFrame*>(lastTransmittedFrame));
    }
    else if (auto blockAck = dynamic_cast<Ieee80211BasicBlockAck *>(frame)) {
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
    else if (dynamic_cast<Ieee80211RTSFrame*>(frame))
        ; // void
    else if (dynamic_cast<Ieee80211CTSFrame*>(frame))
        edcaDataRecoveryProcedures[ac]->ctsFrameReceived();
    else if (frame->getType() == ST_DATA_WITH_QOS)
        ; // void
    else if (dynamic_cast<Ieee80211BasicBlockAckReq*>(frame))
        ; // void
    else
        throw cRuntimeError("Unknown control frame");
}

void Hcf::originatorProcessReceivedDataFrame(Ieee80211DataFrame* frame, Ieee80211Frame* lastTransmittedFrame, AccessCategory ac)
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

void Hcf::sendUp(const std::vector<Ieee80211Frame*>& completeFrames)
{
    for (auto frame : completeFrames) {
        // FIXME: mgmt module does not handle addba req ..
        if (!dynamic_cast<Ieee80211ActionFrame *>(frame))
            mac->sendUp(frame);
    }
}

void Hcf::transmitFrame(Ieee80211Frame* frame, simtime_t ifs)
{
    auto channelOwner = edca->getChannelOwner();
    if (channelOwner) {
        AccessCategory ac = channelOwner->getAccessCategory();
        auto txop = edcaTxops[ac];
        if (auto dataFrame = dynamic_cast<Ieee80211DataFrame*>(frame)) {
            OriginatorBlockAckAgreement *agreement = nullptr;
            if (originatorBlockAckAgreementHandler)
                agreement = originatorBlockAckAgreementHandler->getAgreement(dataFrame->getReceiverAddress(), dataFrame->getTid());
            auto ackPolicy = originatorAckPolicy->computeAckPolicy(dataFrame, agreement);
            dataFrame->setAckPolicy(ackPolicy);
        }
        setFrameMode(frame, rateSelection->computeMode(frame, txop));
        if (txop->getProtectionMechanism() == TxopProcedure::ProtectionMechanism::SINGLE_PROTECTION)
            frame->setDuration(singleProtectionMechanism->computeDurationField(frame, edcaInProgressFrames[ac]->getPendingFrameFor(frame), edcaTxops[ac], recipientAckPolicy));
        else if (txop->getProtectionMechanism() == TxopProcedure::ProtectionMechanism::MULTIPLE_PROTECTION)
            throw cRuntimeError("Multiple protection is unsupported");
        else
            throw cRuntimeError("Undefined protection mechanism");
        tx->transmitFrame(frame, ifs, this);
    }
    else
        throw cRuntimeError("Hcca is unimplemented");
}

void Hcf::transmitControlResponseFrame(Ieee80211Frame* responseFrame, Ieee80211Frame* receivedFrame)
{
    const IIeee80211Mode *responseMode = nullptr;
    if (auto rtsFrame = dynamic_cast<Ieee80211RTSFrame*>(receivedFrame))
        responseMode = rateSelection->computeResponseCtsFrameMode(rtsFrame);
    else if (auto blockAckReq = dynamic_cast<Ieee80211BasicBlockAckReq*>(receivedFrame))
        responseMode = rateSelection->computeResponseBlockAckFrameMode(blockAckReq);
    else if (auto dataOrMgmtFrame = dynamic_cast<Ieee80211DataOrMgmtFrame*>(receivedFrame))
        responseMode = rateSelection->computeResponseAckFrameMode(dataOrMgmtFrame);
    else
        throw cRuntimeError("Unknown received frame type");
    setFrameMode(responseFrame, responseMode);
    tx->transmitFrame(responseFrame, modeSet->getSifsTime(), this);
    delete responseFrame;
}

void Hcf::recipientProcessTransmittedControlResponseFrame(Ieee80211Frame* frame)
{
    if (auto ctsFrame = dynamic_cast<Ieee80211CTSFrame*>(frame))
        ctsProcedure->processTransmittedCts(ctsFrame);
    else if (auto blockAck = dynamic_cast<Ieee80211BlockAck*>(frame)) {
        if (recipientBlockAckProcedure)
            recipientBlockAckProcedure->processTransmittedBlockAck(blockAck);
    }
    else if (auto ackFrame = dynamic_cast<Ieee80211ACKFrame*>(frame))
        recipientAckProcedure->processTransmittedAck(ackFrame);
    else
        throw cRuntimeError("Unknown control response frame");
}


void Hcf::processMgmtFrame(Ieee80211ManagementFrame* mgmtFrame)
{
    processUpperFrame(mgmtFrame);
}

void Hcf::setFrameMode(Ieee80211Frame *frame, const IIeee80211Mode *mode) const
 {
    ASSERT(mode != nullptr);
    delete frame->removeControlInfo();
    Ieee80211TransmissionRequest *ctrl = new Ieee80211TransmissionRequest();
    ctrl->setMode(mode);
    frame->setControlInfo(ctrl);
}


bool Hcf::isReceptionInProgress()
{
    return rx->isReceptionInProgress();
}

bool Hcf::isForUs(Ieee80211Frame *frame) const
{
    return frame->getReceiverAddress() == mac->getAddress() || (frame->getReceiverAddress().isMulticast() && !isSentByUs(frame));
}

bool Hcf::isSentByUs(Ieee80211Frame *frame) const
{
    // FIXME:
    // Check the roles of the Addr3 field when aggregation is applied
    // Table 8-19—Address field contents
    if (auto dataOrMgmtFrame = dynamic_cast<Ieee80211DataOrMgmtFrame *>(frame))
        return dataOrMgmtFrame->getAddress3() == mac->getAddress();
    else
        return false;
}

void Hcf::corruptedFrameReceived()
{
    if (frameSequenceHandler->isSequenceRunning()) {
        if (!startRxTimer->isScheduled())
            frameSequenceHandler->handleStartRxTimeout();
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
