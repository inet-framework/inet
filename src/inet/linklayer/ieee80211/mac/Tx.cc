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
#include "inet/linklayer/ieee80211/mac/contract/IRx.h"
#include "inet/linklayer/ieee80211/mac/contract/IStatistics.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Mac.h"
#include "inet/linklayer/ieee80211/mac/Tx.h"

namespace inet {
namespace ieee80211 {

Define_Module(Tx);

Tx::~Tx()
{
    cancelAndDelete(endIfsTimer);
    if (frame && !transmitting)
        delete frame;
}

void Tx::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        mac = check_and_cast<Ieee80211Mac *>(getContainingNicModule(this)->getSubmodule("mac"));
        endIfsTimer = new cMessage("endIFS");
        rx = dynamic_cast<IRx *>(getModuleByPath(par("rxModule")));
        // statistics = check_and_cast<IStatistics*>(getModuleByPath(par("statisticsModule")));
        WATCH(transmitting);
    }
    if (stage == INITSTAGE_LINK_LAYER) {
        address = mac->getAddress();
        refreshDisplay();
    }
}

void Tx::transmitFrame(Ieee80211Frame *frame, ITx::ICallback *txCallback)
{
    transmitFrame(frame, SIMTIME_ZERO, txCallback);
}

void Tx::transmitFrame(Ieee80211Frame *frame, simtime_t ifs, ITx::ICallback *txCallback)
{
    ASSERT(this->txCallback == nullptr);
    this->txCallback = txCallback;
    Enter_Method("transmitFrame(\"%s\")", frame->getName());
    take(frame);
    auto frameToTransmit = inet::utils::dupPacketAndControlInfo(frame);
    this->frame = frameToTransmit;
    if (auto twoAddrFrame = dynamic_cast<Ieee80211TwoAddressFrame*>(frameToTransmit))
        twoAddrFrame->setTransmitterAddress(address);
    ASSERT(!endIfsTimer->isScheduled() && !transmitting);    // we are idle
    scheduleAt(simTime() + ifs, endIfsTimer);
    if (hasGUI())
        refreshDisplay();
}

void Tx::radioTransmissionFinished()
{
    Enter_Method_Silent();
    if (transmitting) {
        EV_DETAIL << "Tx: radioTransmissionFinished()\n";
        transmitting = false;
        auto transmittedFrame = inet::utils::dupPacketAndControlInfo(frame);
        frame = nullptr;
        ASSERT(txCallback != nullptr);
        ITx::ICallback *tmpTxCallback = txCallback;
        txCallback = nullptr;
        tmpTxCallback->transmissionComplete(transmittedFrame);
        rx->frameTransmitted(durationField);
        if (hasGUI())
            refreshDisplay();
    }
}

void Tx::handleMessage(cMessage *msg)
{
    if (msg == endIfsTimer) {
        EV_DETAIL << "Tx: endIfsTimer expired\n";
        transmitting = true;
        durationField = frame->getDuration();
        mac->sendFrame(frame);
        if (hasGUI())
            refreshDisplay();
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

