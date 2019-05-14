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

#include "inet/common/ModuleAccess.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/ieee80211/mac/channelaccess/Dcaf.h"
#include "inet/linklayer/ieee80211/mac/contract/IRx.h"

namespace inet {
namespace ieee80211 {

using namespace inet::physicallayer;

Define_Module(Dcaf);

void Dcaf::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        getContainingNicModule(this)->subscribe(modesetChangedSignal, this);
    }
    else if (stage == INITSTAGE_LINK_LAYER) {
        // TODO: calculateTimingParameters()
        pendingQueue = check_and_cast<queueing::IPacketQueue *>(getSubmodule("pendingQueue"));
        inProgressFrames = check_and_cast<InProgressFrames *>(getSubmodule("inProgressFrames"));
        contention = check_and_cast<IContention *>(getSubmodule("contention"));
        auto rx = check_and_cast<IRx *>(getModuleByPath(par("rxModule")));
        rx->registerContention(contention);
        calculateTimingParameters();
        WATCH(owning);
        WATCH(slotTime);
        WATCH(sifs);
        WATCH(ifs);
        WATCH(eifs);
        WATCH(cw);
        WATCH(cwMin);
        WATCH(cwMax);
    }
}

void Dcaf::refreshDisplay() const
{
    std::string text;
    if (owning)
        text = "Owning";
    else if (contention->isContentionInProgress())
        text = "Contending";
    else
        text = "Idle";
    getDisplayString().setTagArg("t", 0, text.c_str());
}

void Dcaf::calculateTimingParameters()
{
    slotTime = modeSet->getSlotTime();
    sifs = modeSet->getSifsTime();
    int difsNumber = par("difsn");
    // The PIFS and DIFS are derived by the Equation (9-2) and Equation (9-3), as illustrated in Figure 9-14.
    // PIFS = aSIFSTime + aSlotTime (9-2)
    // DIFS = aSIFSTime + 2 × aSlotTime (9-3)
    ifs = difsNumber == -1 ? sifs + 2 * slotTime : difsNumber * slotTime;
    // The EIFS is derived from the SIFS and the DIFS and the length of time it takes to transmit an ACK frame at the
    // lowest PHY mandatory rate by Equation (9-4).
    // EIFS = aSIFSTime + DIFS + ACKTxTime
    eifs = sifs + ifs + modeSet->getSlowestMandatoryMode()->getDuration(LENGTH_ACK);
    EV_DEBUG << "Timing parameters are initialized: slotTime = " << slotTime << ", sifs = " << sifs << ", ifs = " << ifs << ", eifs = " << eifs << std::endl;
    ASSERT(ifs > sifs);
    cwMin = par("cwMin");
    cwMax = par("cwMax");
    if (cwMin == -1)
        cwMin = modeSet->getCwMin();
    if (cwMax == -1)
        cwMax = modeSet->getCwMax();
    cw = cwMin;
    EV_DEBUG << "Contention window parameters are initialized: cw = " << cw << ", cwMin = " << cwMin << ", cwMax = " << cwMax << std::endl;
}

void Dcaf::incrementCw()
{
    Enter_Method_Silent("incrementCw");
    int newCw = 2 * cw + 1;
    if (newCw > cwMax)
        cw = cwMax;
    else
        cw = newCw;
    EV_DEBUG << "Contention window is incremented: cw = " << cw << std::endl;
}

void Dcaf::resetCw()
{
    Enter_Method_Silent("resetCw");
    cw = cwMin;
    EV_DEBUG << "Contention window is reset: cw = " << cw << std::endl;
}

void Dcaf::channelAccessGranted()
{
    Enter_Method_Silent("channelAccessGranted");
    ASSERT(callback != nullptr);
    owning = true;
    emit(channelOwnershipChangedSignal, owning);
    callback->channelGranted(this);
}

void Dcaf::releaseChannel(IChannelAccess::ICallback* callback)
{
    Enter_Method_Silent("releaseChannel");
    owning = false;
    emit(channelOwnershipChangedSignal, owning);
    this->callback = nullptr;
    EV_INFO << "Channel released.\n";
}

void Dcaf::requestChannel(IChannelAccess::ICallback* callback)
{
    Enter_Method_Silent("requestChannel");
    this->callback = callback;
    if (owning)
        callback->channelGranted(this);
    else if (!contention->isContentionInProgress())
        contention->startContention(cw, ifs, eifs, slotTime, this);
    else
        EV_DEBUG << "Contention has been already started.\n";
}

void Dcaf::expectedChannelAccess(simtime_t time)
{
    // don't care
}

void Dcaf::receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj, cObject* details)
{
    Enter_Method_Silent("receiveSignal");
    if (signalID == modesetChangedSignal) {
        modeSet = check_and_cast<Ieee80211ModeSet*>(obj);
        calculateTimingParameters();
    }
}

} /* namespace ieee80211 */
} /* namespace inet */
