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
        WATCH_EXPR("txState", endIfsTimer != nullptr && endIfsTimer->isScheduled() ? "WAIT_IFS" : transmitting ? "TRANSMIT" : "IDLE");
        WATCH_EXPR("txFramePrefix", frame ? std::string(frame->getName()) + "\n" : "");
    }
}

void Tx::transmitFrame(TxRequestId id, Packet *packet, const Ptr<const Ieee80211MacHeader>& header, simtime_t ifs, ITx::ICallback *txCallback)
{
    Enter_Method("transmitFrame(\"%s\")", packet->getName());
    ASSERT(this->txCallback == nullptr);
    if (id.epoch != lifecycleEpoch || id.serial == 0)
        throw cRuntimeError("Invalid or stale Tx request identity");
    requestId = id;
    this->txCallback = txCallback;
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
    if (ifs == 0) {
        // do directly what handleMessage() would do
        sendPendingFrame();
    }
    else
        scheduleAfter(ifs, endIfsTimer);
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
        auto completedId = requestId;
        CallbackGuard guard(tmpTxCallback);
        frame = nullptr;
        txCallback = nullptr;
        requestId = {};
        tmpTxCallback->transmissionComplete(completedId, tmpFrame, tmpFrame->peekAtFront<Ieee80211MacHeader>());
        delete tmpFrame;
        // Keep response timers ahead of the NAV timer when their deadlines coincide.
        if (completedId.epoch == lifecycleEpoch)
            rx->frameTransmitted(duration);
    }
}

void Tx::handleMessage(cMessage *msg)
{
    if (msg == endIfsTimer) {
        EV_DETAIL << "Tx: endIfsTimer expired\n";
        sendPendingFrame();
    }
    else
        ASSERT(false);
}

ITx::Cancellation Tx::cancelPendingTransmission(TxRequestId id)
{
    Enter_Method("cancelPendingTransmission");
    if (!txCallback || id != requestId)
        return Cancellation::NOT_FOUND;
    if (transmitting)
        return Cancellation::TOO_LATE;
    cancelEvent(endIfsTimer);
    auto canceledFrame = frame;
    frame = nullptr;
    txCallback = nullptr;
    requestId = {};
    delete canceledFrame;
    return Cancellation::CANCELED;
}


void Tx::sendPendingFrame()
{
    auto id = requestId;
    auto callback = txCallback;
    if (!callback)
        return;
    CallbackGuard guard(callback);
    bool permitted = callback->isTransmissionPermitted(id);
    // The callback can cancel this request or replace it synchronously.
    if (requestId != id || txCallback != callback)
        return;
    if (!permitted) {
        if (cancelPendingTransmission(id) == Cancellation::CANCELED)
            callback->transmissionCanceled(id);
        return;
    }
    transmitting = true;
    mac->sendDownFrame(frame->dup());
    if (requestId == id && txCallback == callback && transmitting)
        callback->transmissionStarted(id);
}

} // namespace ieee80211
} // namespace inet
