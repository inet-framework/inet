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

void Tx::transmitFrame(Packet *packet, const Ptr<const Ieee80211MacHeader>& header, bool updateLocalNav, ITx::ICallback *txCallback)
{
    transmitFrame(packet, header, SIMTIME_ZERO, updateLocalNav, txCallback);
}

void Tx::transmitFrame(Packet *packet, const Ptr<const Ieee80211MacHeader>& header, simtime_t ifs, bool updateLocalNav, ITx::ICallback *txCallback)
{
    Enter_Method("transmitFrame(\"%s\")", packet->getName());
    ASSERT(!hasTransmission());
    this->txCallback = txCallback;
    this->updateLocalNav = updateLocalNav;
    finalizeMpdu(packet);
    this->frame = packet->dup();
    ASSERT(!endIfsTimer->isScheduled() && !transmitting);
    if (ifs == 0)
        startTransmission();
    else
        scheduleAfter(ifs, endIfsTimer);
}

void Tx::finalizeMpdu(Packet *packet)
{
    Enter_Method("finalizeMpdu");
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
}

void Tx::transmitAggregate(simtime_t ifs, IAggregateCallback *callback)
{
    Enter_Method("transmitAggregate");
    ASSERT(!hasTransmission() && !endIfsTimer->isScheduled() && !transmitting && !frame);
    aggregateCallback = callback;
    updateLocalNav = false;
    if (ifs == 0)
        startTransmission();
    else
        scheduleAfter(ifs, endIfsTimer);
}

bool Tx::cancelPendingAggregate(IAggregateCallback *callback)
{
    Enter_Method("cancelPendingAggregate");
    if (aggregateCallback != callback || transmitting)
        return false;
    cancelEvent(endIfsTimer);
    aggregateCallback = nullptr;
    return true;
}

void Tx::startTransmission()
{
    if (aggregateCallback) {
        auto packet = aggregateCallback->prepareAggregateTransmission();
        if (!packet) {
            auto callback = aggregateCallback;
            aggregateCallback = nullptr;
            callback->aggregateTransmissionCancelled();
            return;
        }
        frame = packet->dup();
    }
    transmitting = true;
    if (txCallback)
        txCallback->frameTransmissionStarted(frame);
    mac->sendDownFrame(frame->dup());
}

void Tx::radioTransmissionFinished()
{
    Enter_Method("radioTransmissionFinished");
    if (transmitting) {
        EV_DETAIL << "Tx: radioTransmissionFinished()\n";
        transmitting = false;
        if (aggregateCallback) {
            auto completedFrame = frame;
            auto callback = aggregateCallback;
            frame = nullptr;
            aggregateCallback = nullptr;
            callback->transmissionComplete(completedFrame, nullptr);
            delete completedFrame;
            return;
        }
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
        startTransmission();
    }
    else
        ASSERT(false);
}

} // namespace ieee80211
} // namespace inet
