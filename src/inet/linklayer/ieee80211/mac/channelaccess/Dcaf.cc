//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/channelaccess/Dcaf.h"

#include "inet/common/ModuleAccess.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/ieee80211/mac/contract/IRx.h"
#include "inet/networklayer/common/NetworkInterface.h"

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
        // TODO calculateTimingParameters()
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
    else if (contention != nullptr && contention->isContentionInProgress())
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
    Enter_Method("incrementCw");
    int newCw = 2 * cw + 1;
    if (newCw > cwMax)
        cw = cwMax;
    else
        cw = newCw;
    EV_DEBUG << "Contention window is incremented: cw = " << cw << std::endl;
}

void Dcaf::resetCw()
{
    Enter_Method("resetCw");
    cw = cwMin;
    EV_DEBUG << "Contention window is reset: cw = " << cw << std::endl;
}

void Dcaf::channelAccessGranted()
{
    Enter_Method("channelAccessGranted");
    ASSERT(callback != nullptr);
    owning = true;
    emit(channelOwnershipChangedSignal, owning);
    callback->channelGranted(this);
}

void Dcaf::releaseChannel(IChannelAccess::ICallback *callback)
{
    Enter_Method("releaseChannel");
    owning = false;
    emit(channelOwnershipChangedSignal, owning);
    this->callback = nullptr;
    EV_INFO << "Channel released.\n";
}

void Dcaf::requestChannel(IChannelAccess::ICallback *callback)
{
    Enter_Method("requestChannel");
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

void Dcaf::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signalID));

    if (signalID == modesetChangedSignal) {
        modeSet = check_and_cast<Ieee80211ModeSet *>(obj);
        calculateTimingParameters();
    }
}

} /* namespace ieee80211 */
} /* namespace inet */

