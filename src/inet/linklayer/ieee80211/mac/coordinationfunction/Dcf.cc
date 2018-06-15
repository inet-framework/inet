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
#include "inet/linklayer/ieee80211/mac/coordinationfunction/Dcf.h"
#include "inet/linklayer/ieee80211/mac/framesequence/DcfFs.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Mac.h"
#include "inet/linklayer/ieee80211/mac/rateselection/RateSelection.h"
#include "inet/linklayer/ieee80211/mac/recipient/RecipientAckProcedure.h"

namespace inet {
namespace ieee80211 {

using namespace inet::physicallayer;

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
        recoveryProcedure = check_and_cast<NonQosRecoveryProcedure *>(getSubmodule("recoveryProcedure"));
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
        if (!isReceptionInProgress()) {
            frameSequenceHandler->handleStartRxTimeout();
            updateDisplayString();
        }
    }
    else
        throw cRuntimeError("Unknown msg type");
}

void Dcf::updateDisplayString()
{
    if (frameSequenceHandler->isSequenceRunning()) {
        auto history = frameSequenceHandler->getFrameSequence()->getHistory();
        getDisplayString().setTagArg("t", 0, ("Fs: " + history).c_str());
    }
    else
        getDisplayString().removeTag("t");
}

void Dcf::channelGranted(IChannelAccess *channelAccess)
{
    ASSERT(dcfChannelAccess == channelAccess);
    if (!frameSequenceHandler->isSequenceRunning()) {
        frameSequenceHandler->startFrameSequence(new DcfFs(), buildContext(), this);
        updateDisplayString();
    }
}

void Dcf::processUpperFrame(Packet *packet, const Ptr<const Ieee80211DataOrMgmtHeader>& header)
{
    Enter_Method("processUpperFrame(%s)", packet->getName());
    EV_INFO << "Processing upper frame: " << packet->getName() << endl;
    if (pendingQueue->insert(packet)) {
        EV_INFO << "Frame " << packet->getName() << " has been inserted into the PendingQueue." << endl;
        EV_DETAIL << "Requesting channel" << endl;
        dcfChannelAccess->requestChannel(this);
    }
    else {
        EV_INFO << "Frame " << packet->getName() << " has been dropped because the PendingQueue is full." << endl;
        PacketDropDetails details;
        details.setReason(QUEUE_OVERFLOW);
        details.setLimit(pendingQueue->getMaxQueueSize());
        emit(packetDroppedSignal, packet, &details);
        delete packet;
    }
}

void Dcf::transmitControlResponseFrame(Packet *responsePacket, const Ptr<const Ieee80211MacHeader>& responseHeader, Packet *receivedPacket, const Ptr<const Ieee80211MacHeader>& receivedHeader)
{
    responsePacket->insertAtBack(makeShared<Ieee80211MacTrailer>());
    const IIeee80211Mode *responseMode = nullptr;
    if (auto rtsFrame = dynamicPtrCast<const Ieee80211RtsFrame>(receivedHeader))
        responseMode = rateSelection->computeResponseCtsFrameMode(receivedPacket, rtsFrame);
    else if (auto dataOrMgmtHeader = dynamicPtrCast<const Ieee80211DataOrMgmtHeader>(receivedHeader))
        responseMode = rateSelection->computeResponseAckFrameMode(receivedPacket, dataOrMgmtHeader);
    else
        throw cRuntimeError("Unknown received frame type");
    RateSelection::setFrameMode(responsePacket, responseHeader, responseMode);
    tx->transmitFrame(responsePacket, responseHeader, modeSet->getSifsTime(), this);
    delete responsePacket;
}

void Dcf::processMgmtFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& mgmtHeader)
{
    throw cRuntimeError("Unknown management frame");
}

void Dcf::recipientProcessTransmittedControlResponseFrame(Packet *packet, const Ptr<const Ieee80211MacHeader>& header)
{
    mac->emit(packetSentToPeerSignal, packet);
    if (auto ctsFrame = dynamicPtrCast<const Ieee80211CtsFrame>(header))
        ctsProcedure->processTransmittedCts(ctsFrame);
    else if (auto ackFrame = dynamicPtrCast<const Ieee80211AckFrame>(header))
        recipientAckProcedure->processTransmittedAck(ackFrame);
    else
        throw cRuntimeError("Unknown control response frame");
}

void Dcf::scheduleStartRxTimer(simtime_t timeout)
{
    Enter_Method_Silent();
    scheduleAt(simTime() + timeout, startRxTimer);
}

void Dcf::processLowerFrame(Packet *packet, const Ptr<const Ieee80211MacHeader>& header)
{
    Enter_Method_Silent();
    if (frameSequenceHandler->isSequenceRunning()) {
        // TODO: always call processResponses
        if ((!isForUs(header) && !startRxTimer->isScheduled()) || isForUs(header)) {
            frameSequenceHandler->processResponse(packet);
            updateDisplayString();
        }
        else {
            EV_INFO << "This frame is not for us" << std::endl;
            PacketDropDetails details;
            details.setReason(NOT_ADDRESSED_TO_US);
            emit(packetDroppedSignal, packet, &details);
            delete packet;
        }
        cancelEvent(startRxTimer);
    }
    else if (isForUs(header))
        recipientProcessReceivedFrame(packet, header);
    else {
        EV_INFO << "This frame is not for us" << std::endl;
        PacketDropDetails details;
        details.setReason(NOT_ADDRESSED_TO_US);
        emit(packetDroppedSignal, packet, &details);
        delete packet;
    }
}

void Dcf::transmitFrame(Packet *packet, simtime_t ifs)
{
    const auto& header = packet->peekAtFront<Ieee80211MacHeader>();
    RateSelection::setFrameMode(packet, header, rateSelection->computeMode(packet, header));
    auto pendingPacket = inProgressFrames->getPendingFrameFor(packet);
    auto duration = originatorProtectionMechanism->computeDurationField(packet, header, pendingPacket, pendingPacket == nullptr ? nullptr : pendingPacket->peekAtFront<Ieee80211DataOrMgmtHeader>());
    const auto& updatedHeader = packet->removeAtFront<Ieee80211MacHeader>();
    updatedHeader->setDuration(duration);
    packet->insertAtFront(updatedHeader);
    tx->transmitFrame(packet, packet->peekAtFront<Ieee80211MacHeader>(), ifs, this);
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

void Dcf::recipientProcessReceivedFrame(Packet *packet, const Ptr<const Ieee80211MacHeader>& header)
{
    mac->emit(packetReceivedFromPeerSignal, packet);
    if (auto dataOrMgmtHeader = dynamicPtrCast<const Ieee80211DataOrMgmtHeader>(header))
        recipientAckProcedure->processReceivedFrame(packet, dataOrMgmtHeader, recipientAckPolicy, this);
    if (auto dataHeader = dynamicPtrCast<const Ieee80211DataHeader>(header))
        sendUp(recipientDataService->dataFrameReceived(packet, dataHeader));
    else if (auto mgmtHeader = dynamicPtrCast<const Ieee80211MgmtHeader>(header))
        sendUp(recipientDataService->managementFrameReceived(packet, mgmtHeader));
    else { // TODO: else if (auto ctrlFrame = dynamic_cast<Ieee80211ControlFrame*>(frame))
        sendUp(recipientDataService->controlFrameReceived(packet, header));
        recipientProcessControlFrame(packet, header);
        delete packet;
    }
}

void Dcf::sendUp(const std::vector<Packet *>& completeFrames)
{
    for (auto frame : completeFrames)
        mac->sendUpFrame(frame);
}

void Dcf::recipientProcessControlFrame(Packet *packet, const Ptr<const Ieee80211MacHeader>& header)
{
    if (auto rtsFrame = dynamicPtrCast<const Ieee80211RtsFrame>(header))
        ctsProcedure->processReceivedRts(packet, rtsFrame, ctsPolicy, this);
    else
        throw cRuntimeError("Unknown control frame");
}

FrameSequenceContext* Dcf::buildContext()
{
    auto nonQoSContext = new NonQoSContext(originatorAckPolicy);
    return new FrameSequenceContext(mac->getAddress(), modeSet, inProgressFrames, rtsProcedure, rtsPolicy, nonQoSContext, nullptr);
}

void Dcf::transmissionComplete(Packet *packet, const Ptr<const Ieee80211MacHeader>& header)
{
    if (frameSequenceHandler->isSequenceRunning()) {
        frameSequenceHandler->transmissionComplete();
        updateDisplayString();
    }
    else
        recipientProcessTransmittedControlResponseFrame(packet, header);
    delete packet;
}

bool Dcf::hasFrameToTransmit()
{
    return !pendingQueue->isEmpty() || inProgressFrames->hasInProgressFrames();
}

void Dcf::originatorProcessRtsProtectionFailed(Packet *packet)
{
    EV_INFO << "RTS frame transmission failed\n";
    auto protectedHeader = packet->peekAtFront<Ieee80211DataOrMgmtHeader>();
    recoveryProcedure->rtsFrameTransmissionFailed(protectedHeader, stationRetryCounters);
    EV_INFO << "For the current frame exchange, we have CW = " << dcfChannelAccess->getCw() << " SRC = " << recoveryProcedure->getShortRetryCount(packet, protectedHeader) << " LRC = " << recoveryProcedure->getLongRetryCount(packet, protectedHeader) << " SSRC = " << stationRetryCounters->getStationShortRetryCount() << " and SLRC = " << stationRetryCounters->getStationLongRetryCount() << std::endl;
    if (recoveryProcedure->isRtsFrameRetryLimitReached(packet, protectedHeader)) {
        recoveryProcedure->retryLimitReached(packet, protectedHeader);
        inProgressFrames->dropFrame(packet);
        PacketDropDetails details;
        details.setReason(RETRY_LIMIT_REACHED);
        details.setLimit(recoveryProcedure->getShortRetryLimit());
        emit(packetDroppedSignal, packet, &details);
        emit(linkBrokenSignal, packet);
    }
}

void Dcf::originatorProcessTransmittedFrame(Packet *packet)
{
    mac->emit(packetSentToPeerSignal, packet);
    auto transmittedHeader = packet->peekAtFront<Ieee80211MacHeader>();
    if (auto dataOrMgmtHeader = dynamicPtrCast<const Ieee80211DataOrMgmtHeader>(transmittedHeader)) {
        EV_INFO << "For the current frame exchange, we have CW = " << dcfChannelAccess->getCw() << " SRC = " << recoveryProcedure->getShortRetryCount(packet, dataOrMgmtHeader) << " LRC = " << recoveryProcedure->getLongRetryCount(packet, dataOrMgmtHeader) << " SSRC = " << stationRetryCounters->getStationShortRetryCount() << " and SLRC = " << stationRetryCounters->getStationLongRetryCount() << std::endl;
        if (originatorAckPolicy->isAckNeeded(dataOrMgmtHeader)) {
            ackHandler->processTransmittedDataOrMgmtFrame(dataOrMgmtHeader);
        }
        else if (dataOrMgmtHeader->getReceiverAddress().isMulticast()) {
            recoveryProcedure->multicastFrameTransmitted(stationRetryCounters);
            inProgressFrames->dropFrame(packet);
        }
    }
    else if (auto rtsFrame = dynamicPtrCast<const Ieee80211RtsFrame>(transmittedHeader)) {
        auto protectedFrame = inProgressFrames->getFrameToTransmit(); // KLUDGE:
        auto protectedHeader = protectedFrame->peekAtFront<Ieee80211DataOrMgmtHeader>();
        EV_INFO << "For the current frame exchange, we have CW = " << dcfChannelAccess->getCw() << " SRC = " << recoveryProcedure->getShortRetryCount(protectedFrame, protectedHeader) << " LRC = " << recoveryProcedure->getLongRetryCount(protectedFrame, protectedHeader) << " SSRC = " << stationRetryCounters->getStationShortRetryCount() << " and SLRC = " << stationRetryCounters->getStationLongRetryCount() << std::endl;
        rtsProcedure->processTransmittedRts(rtsFrame);
    }
}

void Dcf::originatorProcessReceivedFrame(Packet *receivedPacket, Packet *lastTransmittedPacket)
{
    mac->emit(packetReceivedFromPeerSignal, receivedPacket);
    auto receivedHeader = receivedPacket->peekAtFront<Ieee80211MacHeader>();
    auto lastTransmittedHeader = lastTransmittedPacket->peekAtFront<Ieee80211MacHeader>();
    if (receivedHeader->getType() == ST_ACK) {
        auto lastTransmittedDataOrMgmtHeader = dynamicPtrCast<const Ieee80211DataOrMgmtHeader>(lastTransmittedHeader);
        if (dataAndMgmtRateControl) {
            int retryCount = lastTransmittedHeader->getRetry() ? recoveryProcedure->getRetryCount(lastTransmittedPacket, lastTransmittedDataOrMgmtHeader) : 0;
            dataAndMgmtRateControl->frameTransmitted(lastTransmittedPacket, retryCount, true, false);
        }
        recoveryProcedure->ackFrameReceived(lastTransmittedPacket, lastTransmittedDataOrMgmtHeader, stationRetryCounters);
        ackHandler->processReceivedAck(dynamicPtrCast<const Ieee80211AckFrame>(receivedHeader), lastTransmittedDataOrMgmtHeader);
        inProgressFrames->dropFrame(lastTransmittedPacket);
    }
    else if (receivedHeader->getType() == ST_RTS)
        ; // void
    else if (receivedHeader->getType() == ST_CTS)
        recoveryProcedure->ctsFrameReceived(stationRetryCounters);
    else
        throw cRuntimeError("Unknown frame type");
}

void Dcf::originatorProcessFailedFrame(Packet *failedPacket)
{
    EV_INFO << "Data/Mgmt frame transmission failed\n";
    const auto& failedHeader = failedPacket->peekAtFront<Ieee80211DataOrMgmtHeader>();
    ASSERT(failedHeader->getType() != ST_DATA_WITH_QOS);
    ASSERT(ackHandler->getAckStatus(failedHeader) == AckHandler::Status::WAITING_FOR_ACK);
    recoveryProcedure->dataOrMgmtFrameTransmissionFailed(failedPacket, failedHeader, stationRetryCounters);
    bool retryLimitReached = recoveryProcedure->isRetryLimitReached(failedPacket, failedHeader);
    if (dataAndMgmtRateControl) {
        int retryCount = recoveryProcedure->getRetryCount(failedPacket, failedHeader);
        dataAndMgmtRateControl->frameTransmitted(failedPacket, retryCount, false, retryLimitReached);
    }
    ackHandler->processFailedFrame(failedHeader);
    if (retryLimitReached) {
        recoveryProcedure->retryLimitReached(failedPacket, failedHeader);
        inProgressFrames->dropFrame(failedPacket);
        PacketDropDetails details;
        details.setReason(RETRY_LIMIT_REACHED);
        details.setLimit(-1); // TODO:
        emit(packetDroppedSignal, failedPacket, &details);
        emit(linkBrokenSignal, failedPacket);
    }
    else {
        auto h = failedPacket->removeAtFront<Ieee80211DataOrMgmtHeader>();
        h->setRetry(true);
        failedPacket->insertAtFront(h);
    }
}

bool Dcf::isForUs(const Ptr<const Ieee80211MacHeader>& header) const
{
    return header->getReceiverAddress() == mac->getAddress() || (header->getReceiverAddress().isMulticast() && !isSentByUs(header));
}

bool Dcf::isSentByUs(const Ptr<const Ieee80211MacHeader>& header) const
{
    // FIXME:
    // Check the roles of the Addr3 field when aggregation is applied
    // Table 8-19—Address field contents
    if (auto dataOrMgmtHeader = dynamicPtrCast<const Ieee80211DataOrMgmtHeader>(header))
        return dataOrMgmtHeader->getAddress3() == mac->getAddress();
    else
        return false;
}

void Dcf::corruptedFrameReceived()
{
    if (frameSequenceHandler->isSequenceRunning()) {
        if (!startRxTimer->isScheduled()) {
            frameSequenceHandler->handleStartRxTimeout();
            updateDisplayString();
        }
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

