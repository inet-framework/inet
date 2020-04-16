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
//

#include "inet/common/ModuleAccess.h"
#include "inet/common/Simsignals.h"
#include "inet/linklayer/ieee80211/mac/channelaccess/Edcaf.h"

namespace inet {
namespace ieee80211 {

using namespace inet::physicallayer;

Define_Module(Edcaf);

inline double fallback(double a, double b) {return a!=-1 ? a : b;}
inline simtime_t fallback(simtime_t a, simtime_t b) {return a!=-1 ? a : b;}

Edcaf::~Edcaf()
{
    delete stationRetryCounters;
}

void Edcaf::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        getContainingNicModule(this)->subscribe(modesetChangedSignal, this);
        ac = getAccessCategory(par("accessCategory"));
        contention = check_and_cast<IContention *>(getSubmodule("contention"));
        collisionController = check_and_cast<IEdcaCollisionController *>(getModuleByPath(par("collisionControllerModule")));
        pendingQueue = check_and_cast<queueing::IPacketQueue *>(getSubmodule("pendingQueue"));
        recoveryProcedure = check_and_cast<QosRecoveryProcedure *>(getSubmodule("recoveryProcedure"));
        ackHandler = check_and_cast<QosAckHandler *>(getSubmodule("ackHandler"));
        inProgressFrames = check_and_cast<InProgressFrames *>(getSubmodule("inProgressFrames"));
        txopProcedure = check_and_cast<TxopProcedure *>(getSubmodule("txopProcedure"));
        stationRetryCounters = new StationRetryCounters();
        WATCH(owning);
        WATCH(slotTime);
        WATCH(sifs);
        WATCH(ifs);
        WATCH(eifs);
        WATCH(ac);
        WATCH(cw);
        WATCH(cwMin);
        WATCH(cwMax);
    }
    else if (stage == INITSTAGE_LINK_LAYER) {
        auto rx = check_and_cast<IRx *>(getModuleByPath(par("rxModule")));
        rx->registerContention(contention);
        calculateTimingParameters();
    }
}

void Edcaf::refreshDisplay() const
{
    std::string text(printAccessCategory(ac));
    if (owning)
        text += "\nOwning";
    else if (contention->isContentionInProgress())
        text += "\nContending";
    else
        text += "\nIdle";
    getDisplayString().setTagArg("t", 0, text.c_str());
}

void Edcaf::calculateTimingParameters()
{
    slotTime = modeSet->getSlotTime();
    sifs = modeSet->getSifsTime();
    int aifsn = par("aifsn");
    simtime_t aifs = sifs + fallback(aifsn, getAifsNumber(ac)) * slotTime;
    ifs = aifs;
    eifs = sifs + aifs + modeSet->getSlowestMandatoryMode()->getDuration(LENGTH_ACK);
    EV_DEBUG << "Timing parameters are initialized: slotTime = " << slotTime << ", sifs = " << sifs << ", ifs = " << ifs << ", eifs = " << eifs << std::endl;
    ASSERT(ifs > sifs);
    cwMin = par("cwMin");
    cwMax = par("cwMax");
    if (cwMin == -1)
        cwMin = getCwMin(ac, modeSet->getCwMin());
    if (cwMax == -1)
        cwMax = getCwMax(ac, modeSet->getCwMax(), modeSet->getCwMin());
    cw = cwMin;
    EV_DEBUG << "Contention window parameters are initialized: cw = " << cw << ", cwMin = " << cwMin << ", cwMax = " << cwMax << std::endl;
}


void Edcaf::incrementCw()
{
    Enter_Method_Silent("incrementCw");
    int newCw = 2 * cw + 1;
    if (newCw > cwMax)
        cw = cwMax;
    else
        cw = newCw;
    EV_DEBUG << "Contention window is incremented: cw = " << cw << std::endl;
}

void Edcaf::resetCw()
{
    Enter_Method_Silent("resetCw");
    cw = cwMin;
    EV_DEBUG << "Contention window is reset: cw = " << cw << std::endl;
}

int Edcaf::getAifsNumber(AccessCategory ac)
{
    switch (ac)
    {
        case AC_BK: return 7;
        case AC_BE: return 3;
        case AC_VI: return 2;
        case AC_VO: return 2;
        default: throw cRuntimeError("Unknown access category = %d", ac);
    }
}

AccessCategory Edcaf::getAccessCategory(const char *ac)
{
    if (strcmp("AC_BK", ac) == 0)
        return AC_BK;
    if (strcmp("AC_VI", ac) == 0)
        return AC_VI;
    if (strcmp("AC_VO", ac) == 0)
        return AC_VO;
    if (strcmp("AC_BE", ac) == 0)
        return AC_BE;
    throw cRuntimeError("Unknown Access Category = %s", ac);
}

void Edcaf::channelAccessGranted()
{
    Enter_Method_Silent("channelAccessGranted");
    ASSERT(callback != nullptr);
    if (!collisionController->isInternalCollision(this)) {
        owning = true;
        emit(channelOwnershipChangedSignal, owning);
        callback->channelGranted(this);
    }
    else
        EV_WARN << "Ignoring channel access granted due to internal collision.\n";
}

void Edcaf::releaseChannel(IChannelAccess::ICallback* callback)
{
    Enter_Method_Silent("releaseChannel");
    ASSERT(owning);
    owning = false;
    emit(channelOwnershipChangedSignal, owning);
    this->callback = nullptr;
    EV_INFO << "Channel released.\n";
}

void Edcaf::requestChannel(IChannelAccess::ICallback* callback)
{
    Enter_Method_Silent("requestChannel");
    this->callback = callback;
    ASSERT(!owning);
    if (contention->isContentionInProgress())
        EV_DEBUG << "Contention has been already started.\n";
    else
        contention->startContention(cw, ifs, eifs, slotTime, this);
}

void Edcaf::expectedChannelAccess(simtime_t time)
{
    collisionController->expectedChannelAccess(this, time);
}

bool Edcaf::isInternalCollision()
{
    return collisionController->isInternalCollision(this);
}

int Edcaf::getCwMax(AccessCategory ac, int aCwMax, int aCwMin)
{
    switch (ac)
    {
        case AC_BK: return aCwMax;
        case AC_BE: return aCwMax;
        case AC_VI: return aCwMin;
        case AC_VO: return (aCwMin + 1) / 2 - 1;
        default: throw cRuntimeError("Unknown access category = %d", ac);
    }
}

int Edcaf::getCwMin(AccessCategory ac, int aCwMin)
{
    switch (ac)
    {
        case AC_BK: return aCwMin;
        case AC_BE: return aCwMin;
        case AC_VI: return (aCwMin + 1) / 2 - 1;
        case AC_VO: return (aCwMin + 1) / 4 - 1;
        default: throw cRuntimeError("Unknown access category = %d", ac);
    }
}

void Edcaf::receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj, cObject* details)
{
    Enter_Method_Silent("receiveSignal");
    if (signalID == modesetChangedSignal) {
        modeSet = check_and_cast<Ieee80211ModeSet*>(obj);
        calculateTimingParameters();
    }
}

} // namespace ieee80211
} // namespace inet
