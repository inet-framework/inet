//
// Copyright (C) 2016 OpenSim Ltd.
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
// along with this program; if not, see http://www.gnu.org/licenses/.
//

#include "inet/common/INETUtils.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/checksum/EthernetCRC.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Mac.h"
#include "inet/linklayer/ieee80211/mac/Tx.h"
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
        rx = dynamic_cast<IRx *>(getModuleByPath(par("rxModule")));
        WATCH(transmitting);
    }
}

void Tx::transmitFrame(Packet *packet, const Ptr<const Ieee80211MacHeader>& header, ITx::ICallback *txCallback)
{
    transmitFrame(packet, header, SIMTIME_ZERO, txCallback);
}

void Tx::transmitFrame(Packet *packet, const Ptr<const Ieee80211MacHeader>& header, simtime_t ifs, ITx::ICallback *txCallback)
{
    Enter_Method_Silent("transmitFrame(\"%s\")", packet->getName());
    ASSERT(this->txCallback == nullptr);
    this->txCallback = txCallback;
    take(packet);
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
        auto bufferLength = B(fcsBytes->getChunkLength()).get();
        auto buffer = new uint8_t[bufferLength];
        fcsBytes->copyToBuffer(buffer, bufferLength);
        auto fcs = ethernetCRC(buffer, bufferLength);
        updatedTrailer->setFcs(fcs);
        delete [] buffer;
    }
    packet->insertAtBack(updatedTrailer);
    this->frame = packet->dup();
    ASSERT(!endIfsTimer->isScheduled() && !transmitting);    // we are idle
    if (ifs == 0) {
        // do directly what handleMessage() would do
        transmitting = true;
        mac->sendDownFrame(frame->dup());
    }
    else
        scheduleAt(simTime() + ifs, endIfsTimer);
}

void Tx::radioTransmissionFinished()
{
    Enter_Method_Silent();
    if (transmitting) {
        EV_DETAIL << "Tx: radioTransmissionFinished()\n";
        transmitting = false;
        ASSERT(txCallback != nullptr);
        const auto& header = frame->peekAtFront<Ieee80211MacHeader>();
        auto duration = header->getDurationField();
        auto tmpFrame = frame;
        auto tmpTxCallback = txCallback;
        frame = nullptr;
        txCallback = nullptr;
        tmpTxCallback->transmissionComplete(tmpFrame, tmpFrame->peekAtFront<Ieee80211MacHeader>());
        rx->frameTransmitted(duration);
    }
}

void Tx::handleMessage(cMessage *msg)
{
    if (msg == endIfsTimer) {
        EV_DETAIL << "Tx: endIfsTimer expired\n";
        transmitting = true;
        mac->sendDownFrame(frame->dup());
    }
    else
        ASSERT(false);
}

void Tx::refreshDisplay() const
{
    const char *stateName = endIfsTimer->isScheduled() ? "WAIT_IFS" : transmitting ? "TRANSMIT" : "IDLE";
    // faster version is just to display the state: getDisplayString().setTagArg("t", 0, stateName);
    std::stringstream os;
    if (frame)
        os << frame->getName() << "\n";
    os << stateName;
    getDisplayString().setTagArg("t", 0, os.str().c_str());
}

} // namespace ieee80211
} // namespace inet

