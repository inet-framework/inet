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
#include "inet/linklayer/ieee80211/mac/coordinationfunction/Dcf.h"
#include "inet/linklayer/ieee80211/mac/framesequence/DcfFs.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Mac.h"
#include "inet/linklayer/ieee80211/mac/rateselection/RateSelection.h"
#include "inet/linklayer/ieee80211/mac/recipient/RecipientAckProcedure.h"

namespace inet {
namespace ieee80211 {

Define_Module(Dcf);

void Dcf::initialize(int stage)
{
    ModeSetListener::initialize(stage);
    if (stage == INITSTAGE_LINK_LAYER_2) {
        startRxTimer = new cMessage("startRxTimeout");
        mac = check_and_cast<Ieee80211Mac *>(getContainingNicModule(this)->getSubmodule("mac"));
        dataAndMgmtRateControl = dynamic_cast<IRateControl *>(getSubmodule(("rateControl")));
        tx = check_and_cast<ITx *>(getModuleByPath(par("txModule")));
        rx = check_and_cast<IRx *>(getModuleByPath(par("rxModule")));
        dcfChannelAccess = check_and_cast<Dcaf *>(getSubmodule("channelAccess"));
        originatorDataService = check_and_cast<IOriginatorMacDataService *>(getSubmodule(("originatorMacDataService")));
        recipientDataService = check_and_cast<IRecipientMacDataService*>(getSubmodule("recipientMacDataService"));
        recoveryProcedure = check_and_cast<NonQoSRecoveryProcedure *>(getSubmodule("recoveryProcedure"));
        rateSelection = check_and_cast<IRateSelection*>(getSubmodule("rateSelection"));
        pendingQueue = new PendingQueue(par("maxQueueSize"), nullptr, par("prioritizeMulticast") ? PendingQueue::Priority::PRIORITIZE_MULTICAST_OVER_DATA : PendingQueue::Priority::PRIORITIZE_MGMT_OVER_DATA);
        rtsProcedure = new RtsProcedure();
        rtsPolicy = check_and_cast<IRtsPolicy *>(getSubmodule("rtsPolicy"));
        recipientAckProcedure = new RecipientAckProcedure();
        recipientAckPolicy = check_and_cast<IRecipientAckPolicy *>(getSubmodule("recipientAckPolicy"));
        originatorAckPolicy = check_and_cast<IOriginatorAckPolicy *>(getSubmodule("originatorAckPolicy"));
        frameSequenceHandler = new FrameSequenceHandler();
        ackHandler = new AckHandler();
        ctsProcedure = new CtsProcedure();
        ctsPolicy = check_and_cast<ICtsPolicy *>(getSubmodule("ctsPolicy"));
        stationRetryCounters = new StationRetryCounters();
        inProgressFrames = new InProgressFrames(pendingQueue, originatorDataService, ackHandler);
        originatorProtectionMechanism = check_and_cast<OriginatorProtectionMechanism*>(getSubmodule("originatorProtectionMechanism"));
    }
}

void Dcf::handleMessage(cMessage* msg)
{
    if (msg == startRxTimer) {
        if (!isReceptionInProgress())
            frameSequenceHandler->handleStartRxTimeout();
    }
    else
        throw cRuntimeError("Unknown msg type");
}

void Dcf::channelGranted(IChannelAccess *channelAccess)
{
    ASSERT(dcfChannelAccess == channelAccess);
    if (!frameSequenceHandler->isSequenceRunning())
        frameSequenceHandler->startFrameSequence(new DcfFs(), buildContext(), this);
}

void Dcf::processUpperFrame(Ieee80211DataOrMgmtFrame* frame)
{
    Enter_Method("processUpperFrame(%s)", frame->getName());
    EV_INFO << "Processing upper frame: " << frame->getName() << endl;
    if (pendingQueue->insert(frame)) {
        EV_INFO << "Frame " << frame->getName() << " has been inserted into the PendingQueue." << endl;
        EV_DETAIL << "Requesting channel" << endl;
        dcfChannelAccess->requestChannel(this);
    }
    else {
        EV_INFO << "Frame " << frame->getName() << " has been dropped because the PendingQueue is full." << endl;
        emit(NF_PACKET_DROP, frame);
        delete frame;
    }
}

void Dcf::transmitControlResponseFrame(Ieee80211Frame* responseFrame, Ieee80211Frame* receivedFrame)
{
    const IIeee80211Mode *responseMode = nullptr;
    if (auto rtsFrame = dynamic_cast<Ieee80211RTSFrame*>(receivedFrame))
        responseMode = rateSelection->computeResponseCtsFrameMode(rtsFrame);
    else if (auto dataOrMgmtFrame = dynamic_cast<Ieee80211DataOrMgmtFrame*>(receivedFrame))
        responseMode = rateSelection->computeResponseAckFrameMode(dataOrMgmtFrame);
    else
        throw cRuntimeError("Unknown received frame type");
    RateSelection::setFrameMode(responseFrame, responseMode);
    tx->transmitFrame(responseFrame, modeSet->getSifsTime(), this);
    delete responseFrame;
}

void Dcf::processMgmtFrame(Ieee80211ManagementFrame* mgmtFrame)
{
    throw cRuntimeError("Unknown management frame");
}

void Dcf::recipientProcessTransmittedControlResponseFrame(Ieee80211Frame* frame)
{
    if (auto ctsFrame = dynamic_cast<Ieee80211CTSFrame*>(frame))
        ctsProcedure->processTransmittedCts(ctsFrame);
    else if (auto ackFrame = dynamic_cast<Ieee80211ACKFrame*>(frame))
        recipientAckProcedure->processTransmittedAck(ackFrame);
    else
        throw cRuntimeError("Unknown control response frame");
}

void Dcf::scheduleStartRxTimer(simtime_t timeout)
{
    Enter_Method_Silent();
    scheduleAt(simTime() + timeout, startRxTimer);
}

void Dcf::processLowerFrame(Ieee80211Frame* frame)
{
    Enter_Method_Silent();
    if (frameSequenceHandler->isSequenceRunning()) {
        // TODO: always call processResponses
        if ((!isForUs(frame) && !startRxTimer->isScheduled()) || isForUs(frame)) {
            frameSequenceHandler->processResponse(frame);
        }
        else {
            EV_INFO << "This frame is not for us" << std::endl;
            delete frame;
        }
        cancelEvent(startRxTimer);
    }
    else if (isForUs(frame))
        recipientProcessReceivedFrame(frame);
    else {
        EV_INFO << "This frame is not for us" << std::endl;
        delete frame;
    }
}

void Dcf::transmitFrame(Ieee80211Frame* frame, simtime_t ifs)
{
    RateSelection::setFrameMode(frame, rateSelection->computeMode(frame));
    frame->setDuration(originatorProtectionMechanism->computeDurationField(frame, inProgressFrames->getPendingFrameFor(frame)));
    tx->transmitFrame(frame, ifs, this);
}

/*
 * TODO:  If a PHY-RXSTART.indication primitive does not occur during the ACKTimeout interval,
 * the STA concludes that the transmission of the MPDU has failed, and this STA shall invoke its
 * backoff procedure **upon expiration of the ACKTimeout interval**.
 */

void Dcf::frameSequenceFinished()
{
    dcfChannelAccess->releaseChannel(this);
    if (hasFrameToTransmit())
        dcfChannelAccess->requestChannel(this);
    mac->sendDownPendingRadioConfigMsg(); // TODO: review
}

bool Dcf::isReceptionInProgress()
{
    return rx->isReceptionInProgress();
}

void Dcf::recipientProcessReceivedFrame(Ieee80211Frame* frame)
{
    if (auto dataOrMgmtFrame = dynamic_cast<Ieee80211DataOrMgmtFrame *>(frame))
        recipientAckProcedure->processReceivedFrame(dataOrMgmtFrame, recipientAckPolicy, this);
    if (auto dataFrame = dynamic_cast<Ieee80211DataFrame*>(frame))
        sendUp(recipientDataService->dataFrameReceived(dataFrame));
    else if (auto mgmtFrame = dynamic_cast<Ieee80211ManagementFrame*>(frame))
        sendUp(recipientDataService->managementFrameReceived(mgmtFrame));
    else { // TODO: else if (auto ctrlFrame = dynamic_cast<Ieee80211ControlFrame*>(frame))
        sendUp(recipientDataService->controlFrameReceived(frame));
        recipientProcessControlFrame(frame);
        delete frame;
    }
}

void Dcf::sendUp(const std::vector<Ieee80211Frame*>& completeFrames)
{
    for (auto frame : completeFrames)
        mac->sendUp(frame);
}

void Dcf::recipientProcessControlFrame(Ieee80211Frame* frame)
{
    if (auto rtsFrame = dynamic_cast<Ieee80211RTSFrame *>(frame))
        ctsProcedure->processReceivedRts(rtsFrame, ctsPolicy, this);
    else
        throw cRuntimeError("Unknown control frame");
}

FrameSequenceContext* Dcf::buildContext()
{
    auto nonQoSContext = new NonQoSContext(originatorAckPolicy);
    return new FrameSequenceContext(mac->getAddress(), modeSet, inProgressFrames, rtsProcedure, rtsPolicy, nonQoSContext, nullptr);
}

void Dcf::transmissionComplete(Ieee80211Frame *frame)
{
    if (frameSequenceHandler->isSequenceRunning())
        frameSequenceHandler->transmissionComplete();
    else
        recipientProcessTransmittedControlResponseFrame(frame);
    delete frame;
}

bool Dcf::hasFrameToTransmit()
{
    return !pendingQueue->isEmpty() || inProgressFrames->hasInProgressFrames();
}

void Dcf::originatorProcessRtsProtectionFailed(Ieee80211DataOrMgmtFrame* protectedFrame)
{
    EV_INFO << "RTS frame transmission failed\n";
    recoveryProcedure->rtsFrameTransmissionFailed(protectedFrame, stationRetryCounters);
    EV_INFO << "For the current frame exchange, we have CW = " << dcfChannelAccess->getCw() << " SRC = " << recoveryProcedure->getShortRetryCount(protectedFrame) << " LRC = " << recoveryProcedure->getLongRetryCount(protectedFrame) << " SSRC = " << stationRetryCounters->getStationShortRetryCount() << " and SLRC = " << stationRetryCounters->getStationLongRetryCount() << std::endl;
    if (recoveryProcedure->isRtsFrameRetryLimitReached(protectedFrame)) {
        emit(NF_LINK_BREAK, protectedFrame);
        recoveryProcedure->retryLimitReached(protectedFrame);
        inProgressFrames->dropFrame(protectedFrame);
        emit(NF_PACKET_DROP, protectedFrame);
    }
}

void Dcf::originatorProcessTransmittedFrame(Ieee80211Frame* transmittedFrame)
{
    if (auto dataOrMgmtFrame = dynamic_cast<Ieee80211DataOrMgmtFrame *>(transmittedFrame)) {
        EV_INFO << "For the current frame exchange, we have CW = " << dcfChannelAccess->getCw() << " SRC = " << recoveryProcedure->getShortRetryCount(dataOrMgmtFrame) << " LRC = " << recoveryProcedure->getLongRetryCount(dataOrMgmtFrame) << " SSRC = " << stationRetryCounters->getStationShortRetryCount() << " and SLRC = " << stationRetryCounters->getStationLongRetryCount() << std::endl;
        if (originatorAckPolicy->isAckNeeded(dataOrMgmtFrame)) {
            ackHandler->processTransmittedDataOrMgmtFrame(dataOrMgmtFrame);
        }
        else if (dataOrMgmtFrame->getReceiverAddress().isMulticast()) {
            recoveryProcedure->multicastFrameTransmitted(stationRetryCounters);
            inProgressFrames->dropFrame(dataOrMgmtFrame);
        }
    }
    else if (auto rtsFrame = dynamic_cast<Ieee80211RTSFrame *>(transmittedFrame)) {
        auto protectedFrame = inProgressFrames->getFrameToTransmit(); // TODO: kludge
        EV_INFO << "For the current frame exchange, we have CW = " << dcfChannelAccess->getCw() << " SRC = " << recoveryProcedure->getShortRetryCount(protectedFrame) << " LRC = " << recoveryProcedure->getLongRetryCount(protectedFrame) << " SSRC = " << stationRetryCounters->getStationShortRetryCount() << " and SLRC = " << stationRetryCounters->getStationLongRetryCount() << std::endl;
        rtsProcedure->processTransmittedRts(rtsFrame);
    }
}

void Dcf::originatorProcessReceivedFrame(Ieee80211Frame* frame, Ieee80211Frame* lastTransmittedFrame)
{
    if (frame->getType() == ST_ACK) {
        auto lastTransmittedDataOrMgmtFrame = check_and_cast<Ieee80211DataOrMgmtFrame*>(lastTransmittedFrame);
        if (dataAndMgmtRateControl) {
            int retryCount = lastTransmittedFrame->getRetry() ? recoveryProcedure->getRetryCount(lastTransmittedDataOrMgmtFrame) : 0;
            dataAndMgmtRateControl->frameTransmitted(frame, retryCount, true, false);
        }
        recoveryProcedure->ackFrameReceived(lastTransmittedDataOrMgmtFrame, stationRetryCounters);
        ackHandler->processReceivedAck(check_and_cast<Ieee80211ACKFrame *>(frame), lastTransmittedDataOrMgmtFrame);
        inProgressFrames->dropFrame(lastTransmittedDataOrMgmtFrame);
    }
    else if (frame->getType() == ST_RTS)
        ; // void
    else if (frame->getType() == ST_CTS)
        recoveryProcedure->ctsFrameReceived(stationRetryCounters);
    else
        throw cRuntimeError("Unknown frame type");
}

void Dcf::originatorProcessFailedFrame(Ieee80211DataOrMgmtFrame* failedFrame)
{
    ASSERT(failedFrame->getType() != ST_DATA_WITH_QOS);
    ASSERT(ackHandler->getAckStatus(failedFrame) == AckHandler::Status::WAITING_FOR_ACK);
    EV_INFO << "Data/Mgmt frame transmission failed\n";
    recoveryProcedure->dataOrMgmtFrameTransmissionFailed(failedFrame, stationRetryCounters);
    bool retryLimitReached = recoveryProcedure->isRetryLimitReached(failedFrame);
    if (dataAndMgmtRateControl) {
        int retryCount = recoveryProcedure->getRetryCount(failedFrame);
        dataAndMgmtRateControl->frameTransmitted(failedFrame, retryCount, false, retryLimitReached);
    }
    ackHandler->processFailedFrame(failedFrame);
    if (retryLimitReached) {
        emit(NF_LINK_BREAK, failedFrame);
        recoveryProcedure->retryLimitReached(failedFrame);
        inProgressFrames->dropFrame(failedFrame);
        emit(NF_PACKET_DROP, failedFrame);
    }
    else
        failedFrame->setRetry(true);
}

bool Dcf::isForUs(Ieee80211Frame *frame) const
{
    return frame->getReceiverAddress() == mac->getAddress() || (frame->getReceiverAddress().isMulticast() && !isSentByUs(frame));
}

bool Dcf::isSentByUs(Ieee80211Frame *frame) const
{
    // FIXME:
    // Check the roles of the Addr3 field when aggregation is applied
    // Table 8-19—Address field contents
    if (auto dataOrMgmtFrame = dynamic_cast<Ieee80211DataOrMgmtFrame *>(frame))
        return dataOrMgmtFrame->getAddress3() == mac->getAddress();
    else
        return false;
}

void Dcf::corruptedFrameReceived()
{
    if (frameSequenceHandler->isSequenceRunning()) {
        if (!startRxTimer->isScheduled())
            frameSequenceHandler->handleStartRxTimeout();
    }
}

Dcf::~Dcf()
{
    cancelAndDelete(startRxTimer);
    delete rtsProcedure;
    delete recipientAckProcedure;
    delete ackHandler;
    delete stationRetryCounters;
    delete ctsProcedure;
    delete frameSequenceHandler;
    delete inProgressFrames;
    delete pendingQueue;
}

} // namespace ieee80211
} // namespace inet

