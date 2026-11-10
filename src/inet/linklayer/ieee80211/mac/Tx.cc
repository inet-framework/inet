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
#include "inet/linklayer/ieee80211/mac/aggregation/Ieee80211Ampdu.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Tag_m.h"

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
        WATCH_EXPR("txState", endIfsTimer != nullptr && endIfsTimer->isScheduled() ? "WAIT_IFS" : transmitting ? "TRANSMIT" : frame ? "WAIT_READY" : "IDLE");
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
    ASSERT(this->txCallback == nullptr);
    this->txCallback = txCallback;
    this->updateLocalNav = updateLocalNav;
    auto mode = packet->findTag<physicallayer::Ieee80211ModeReq>();
    if (mode && mode->getTxVector().ampdu) {
        Ieee80211AmpduLayout layout;
        bool vht = mode->getTxVector().format == physicallayer::Ieee80211TxVector::VHT;
        if (!findAmpdu(packet, b(0), B(packet->getDataLength()), vht, layout))
            throw cRuntimeError("Invalid outgoing A-MPDU layout");
        std::vector<std::unique_ptr<Packet>> copies;
        std::vector<Packet *> members;
        for (const auto& item : layout.members) {
            auto member = std::make_unique<Packet>("A-MPDU member", packet->peekDataAt(item.offset, item.length));
            member->copyTags(*packet);
            member->copyRegionTags(*packet, packet->getFrontOffset()+item.offset, b(0), item.length);
            finalizeMpdu(member.get());
            members.push_back(member.get());
            copies.push_back(std::move(member));
        }
        // Retain the caller's delimiter and spacing bytes; replace only members.
        for (size_t i = 0; i < layout.members.size(); i++)
            packet->replaceAt(members[i]->peekData(), layout.members[i].offset, layout.members[i].length);
        completionHeader = members.front()->peekAtFront<Ieee80211MacHeader>();
    }
    else {
        finalizeMpdu(packet);
        completionHeader = nullptr;
    }
    this->frame = packet->dup();
    admittedStart = simTime() + ifs;
    ASSERT(!endIfsTimer->isScheduled() && !transmitting);
    if (mac->defersTransmissionStart()) {
        if (ifs > 0)
            scheduleAt(admittedStart, endIfsTimer);
        mac->requestTransmissionReadiness();
        return;
    }
    if (ifs == 0) {
        transmitting = true;
        mac->sendDownFrame(frame->dup());
    }
    else
        scheduleAfter(ifs, endIfsTimer);
}

void Tx::finalizeMpdu(Packet *packet)
{
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

bool Tx::cancelPendingTransmission(ITx::ICallback *callback)
{
    Enter_Method("cancelPendingTransmission");
    if (callback != txCallback || !frame || transmitting)
        return false;
    if (endIfsTimer->isScheduled()) cancelEvent(endIfsTimer);
    auto old = frame;
    frame = nullptr;
    txCallback = nullptr;
    updateLocalNav = false;
    completionHeader = nullptr;
    admittedStart = -1;
    delete old;
    return true;
}

void Tx::cancelUnstartedTransmission()
{
    ASSERT(frame && txCallback && !transmitting);
    if (endIfsTimer->isScheduled()) cancelEvent(endIfsTimer);
    std::unique_ptr<Packet> old(frame);
    auto callback = txCallback;
    frame = nullptr;
    txCallback = nullptr;
    completionHeader = nullptr;
    updateLocalNav = false;
    admittedStart = -1;
    // The callback may finish/delete the sequence and submit another frame.
    callback->transmissionCancelled(old.get());
}

void Tx::resumePendingTransmission()
{
    Enter_Method("resumePendingTransmission");
    if (!frame || !txCallback || transmitting || simTime() < admittedStart)
        return;
    if (simTime() > admittedStart) {
        cancelUnstartedTransmission();
        return;
    }
    if (!mac->isTransmissionReady()) {
        // An equal-now physical completion must run before readiness exists.
        // Its mode notification schedules the continuation; never poll here.
        if (!mac->isRadioReadyForTransmission(admittedStart))
            cancelUnstartedTransmission();
        return;
    }
    auto callback = txCallback;
    auto id = frame->getId();
    bool accepted = callback->prepareTransmissionStart(frame, admittedStart);
    if (!frame || frame->getId() != id || txCallback != callback)
        return;
    if (!accepted) {
        cancelUnstartedTransmission();
        return;
    }
    transmitting = true;
    mac->sendDownFrame(frame->dup());
}

void Tx::radioTransmissionFinished()
{
    Enter_Method("radioTransmissionFinished");
    if (transmitting) {
        EV_DETAIL << "Tx: radioTransmissionFinished()\n";
        transmitting = false;
        ASSERT(txCallback != nullptr);
        auto mode = frame->findTag<physicallayer::Ieee80211ModeReq>();
        auto header = mode && mode->getTxVector().ampdu ? completionHeader : frame->peekAtFront<Ieee80211MacHeader>();
        auto duration = header->getDurationField();
        auto tmpFrame = frame;
        auto tmpTxCallback = txCallback;
        auto completedUpdateLocalNav = updateLocalNav;
        frame = nullptr;
        txCallback = nullptr;
        updateLocalNav = false;
        completionHeader = nullptr;
        admittedStart = -1;
        tmpTxCallback->transmissionComplete(tmpFrame, header);
        delete tmpFrame;
        if (completedUpdateLocalNav)
            rx->frameTransmitted(duration);
    }
}

void Tx::handleMessage(cMessage *msg)
{
    if (msg == endIfsTimer) {
        EV_DETAIL << "Tx: endIfsTimer expired\n";
        if (mac->defersTransmissionStart()) {
            mac->requestTransmissionReadiness();
            return;
        }
        transmitting = true;
        mac->sendDownFrame(frame->dup());
    }
    else
        ASSERT(false);
}

} // namespace ieee80211
} // namespace inet
