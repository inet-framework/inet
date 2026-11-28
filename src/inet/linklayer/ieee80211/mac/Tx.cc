//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/Tx.h"

#include "inet/common/INETUtils.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/checksum/Checksum.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Mac.h"
#include "inet/linklayer/ieee80211/mac/contract/IRx.h"

namespace inet {
namespace ieee80211 {

Define_Module(Tx);

Tx::~Tx()
{
    cancelAndDelete(endIfsTimer);
    cancelAndDelete(startDeadlineTimer);
    if (frame)
        delete frame;
}

void Tx::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        mac = check_and_cast<Ieee80211Mac *>(getContainingNicModule(this)->getSubmodule("mac"));
        endIfsTimer = new cMessage("endIFS");
        rx = dynamic_cast<IRx *>(findModuleByPath(par("rxModule")));
        WATCH(transmitting);
        WATCH_EXPR("txState", endIfsTimer != nullptr && endIfsTimer->isScheduled() ? "WAIT_IFS" : transmitting ? "TRANSMIT" : frame ? "WAIT_RADIO" : "IDLE");
        WATCH_EXPR("txFramePrefix", frame ? std::string(frame->getName()) + "\n" : "");
    }
}

void Tx::transmitFrame(Packet *packet, const Ptr<const Ieee80211MacHeader>& header, bool updateLocalNav, ITx::ICallback *txCallback, int64_t *requestId)
{
    transmitFrame(packet, header, SIMTIME_ZERO, updateLocalNav, txCallback, requestId);
}

void Tx::transmitFrame(Packet *packet, const Ptr<const Ieee80211MacHeader>& header, simtime_t ifs, bool updateLocalNav, ITx::ICallback *txCallback, int64_t *requestId)
{
    Enter_Method("transmitFrame(\"%s\")", packet->getName());
    ASSERT(this->txCallback == nullptr);
    this->txCallback = txCallback;
    this->updateLocalNav = updateLocalNav;
    auto macAddressInd = packet->addTagIfAbsent<MacAddressInd>();
    const auto& updatedHeader = packet->removeAtFront<Ieee80211MacHeader>();
    if (auto oneAddressHeader = dynamicPtrCast<Ieee80211OneAddressHeader>(updatedHeader)) {
        macAddressInd->setDestAddress(oneAddressHeader->getReceiverAddress());
    }
    if (auto twoAddressHeader = dynamicPtrCast<Ieee80211TwoAddressHeader>(updatedHeader)) {
        twoAddressHeader->setTransmitterAddress(mac->getAddress());
        macAddressInd->setSrcAddress(twoAddressHeader->getTransmitterAddress());
    }
    packet->insertAtFront(updatedHeader);
    const auto& updatedTrailer = packet->removeAtBack<Ieee80211MacTrailer>(B(4));
    updatedTrailer->setFcsMode(mac->getFcsMode());
    if (mac->getFcsMode() == FCS_COMPUTED) {
        const auto& fcsBytes = packet->peekAllAsBytes();
        auto bufferLength = fcsBytes->getChunkLength().get<B>();
        auto buffer = new uint8_t[bufferLength];
        fcsBytes->copyToBuffer(buffer, bufferLength);
        auto fcs = ethernetFcs(buffer, bufferLength);
        updatedTrailer->setFcs(fcs);
        delete[] buffer;
    }
    packet->insertAtBack(updatedTrailer);
    this->frame = packet->dup();
    ASSERT(!endIfsTimer->isScheduled() && !transmitting); // we are idle
    auto id = frame->getId();
    if (requestId)
        *requestId = id;
    transmissionStartTime = simTime() + ifs;
    // The MAC may withdraw or replace this request inside the start hook.
    bool deferred = mac->shouldDeferTransmissionStart(id, transmissionStartTime);
    if (!frame || frame->getId() != id || transmitting)
        return;
    if (deferred) {
        if (!startDeadlineTimer) {
            startDeadlineTimer = new cMessage("startDeadline");
            // Give the MAC readiness event at the admitted start time the first chance.
            startDeadlineTimer->setSchedulingPriority(std::numeric_limits<short>::max());
        }
        scheduleAt(transmissionStartTime, startDeadlineTimer);
    }
    else if (ifs == 0)
        startPendingTransmission(id);
    else
        scheduleAt(transmissionStartTime, endIfsTimer);
}

bool Tx::startPendingTransmission(int64_t requestId)
{
    Enter_Method("startPendingTransmission");
    if (!frame || frame->getId() != requestId || transmitting || endIfsTimer->isScheduled() || simTime() != transmissionStartTime)
        return false;
    if (startDeadlineTimer)
        cancelEvent(startDeadlineTimer);
    transmitting = true;
    txCallback->transmissionStarted(*frame);
    mac->sendDownFrame(frame->dup());
    return true;
}

bool Tx::cancelPendingTransmission(int64_t requestId)
{
    Enter_Method("cancelPendingTransmission");
    if (!frame || frame->getId() != requestId || transmitting)
        return false;
    cancelEvent(endIfsTimer);
    if (startDeadlineTimer)
        cancelEvent(startDeadlineTimer);
    clearPendingTransmission();
    mac->notifyTransmissionWithdrawn(requestId);
    mac->sendDownPendingRadioConfigMsg();
    return true;
}

void Tx::clearPendingTransmission()
{
    delete frame;
    frame = nullptr;
    txCallback = nullptr;
    updateLocalNav = false;
}

void Tx::radioTransmissionFinished()
{
    Enter_Method("radioTransmissionFinished");
    if (transmitting) {
        EV_DETAIL << "Tx: radioTransmissionFinished()\n";
        transmitting = false;
        ASSERT(txCallback != nullptr);
        const auto& header = frame->peekAtFront<Ieee80211MacHeader>();
        auto duration = header->getDurationField();
        auto tmpFrame = frame;
        auto tmpTxCallback = txCallback;
        auto completedUpdateLocalNav = updateLocalNav;
        frame = nullptr;
        txCallback = nullptr;
        updateLocalNav = false;
        tmpTxCallback->transmissionComplete(tmpFrame, tmpFrame->peekAtFront<Ieee80211MacHeader>());
        delete tmpFrame;
        if (completedUpdateLocalNav)
            rx->frameTransmitted(duration);
    }
}

void Tx::handleMessage(cMessage *msg)
{
    if (msg == endIfsTimer) {
        EV_DETAIL << "Tx: endIfsTimer expired\n";
        startPendingTransmission(frame->getId());
    }
    else if (msg == startDeadlineTimer) {
        auto callback = txCallback;
        auto requestId = frame->getId();
        clearPendingTransmission();
        mac->notifyTransmissionWithdrawn(requestId);
        mac->sendDownPendingRadioConfigMsg();
        callback->transmissionStartFailed();
    }
    else
        ASSERT(false);
}

} // namespace ieee80211
} // namespace inet
