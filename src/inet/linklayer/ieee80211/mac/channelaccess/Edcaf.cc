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

#include "Edcaf.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/NotifierConsts.h"

namespace inet {
namespace ieee80211 {

Define_Module(Edcaf);

inline double fallback(double a, double b) {return a!=-1 ? a : b;}
inline simtime_t fallback(simtime_t a, simtime_t b) {return a!=-1 ? a : b;}

void Edcaf::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        getContainingNicModule(this)->subscribe(NF_MODESET_CHANGED, this);
        ac = getAccessCategory(par("accessCategory"));
        contention = check_and_cast<IContention *>(getSubmodule("contention"));
        collisionController = check_and_cast<IEdcaCollisionController *>(getModuleByPath(par("collisionControllerModule")));
    }
    else if (stage == INITSTAGE_LINK_LAYER_2) {
        auto rx = check_and_cast<IRx *>(getModuleByPath(par("rxModule")));
        rx->registerContention(contention);
        calculateTimingParameters();
        if (hasGUI())
            updateDisplayString();
    }
}

void Edcaf::calculateTimingParameters()
{
    slotTime = modeSet->getSlotTime();
    sifs = modeSet->getSifsTime();
    int aifsn = par("aifsn");
    simtime_t aifs = sifs + fallback(aifsn, getAifsNumber(ac)) * slotTime;
    ifs = aifs;
    eifs = sifs + aifs + modeSet->getSlowestMandatoryMode()->getDuration(LENGTH_ACK);
    ASSERT(ifs > sifs);
    cwMin = par("cwMin");
    cwMax = par("cwMax");
    if (cwMin == -1)
        cwMin = getCwMin(ac, modeSet->getCwMin());
    if (cwMax == -1)
        cwMax = getCwMax(ac, modeSet->getCwMax(), modeSet->getCwMin());
    cw = cwMin;
}


void Edcaf::incrementCw()
{
    int newCw = 2 * cw + 1;
    if (newCw > cwMax)
        cw = cwMax;
    else
        cw = newCw;
}

void Edcaf::resetCw()
{
    cw = cwMin;
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
    ASSERT(callback != nullptr);
    if (!collisionController->isInternalCollision(this)) {
        owning = true;
        callback->channelGranted(this);
    }
    if (hasGUI())
        updateDisplayString();
}

void Edcaf::releaseChannel(IChannelAccess::ICallback* callback)
{
    ASSERT(owning);
    owning = false;
    this->callback = nullptr;
    if (hasGUI())
        updateDisplayString();
}

void Edcaf::requestChannel(IChannelAccess::ICallback* callback)
{
    this->callback = callback;
    ASSERT(!owning);
    if (contention->isContentionInProgress())
        EV_DETAIL << "Contention has already been started" << std::endl;
    else {
//        EV_DETAIL << "Starting contention with cw = " << cw << ", ifs = " << ifs << ", eifs = "
//                  << eifs << ", slotTime = " << slotTime << std::endl;
        contention->startContention(cw, ifs, eifs, slotTime, this);
    }
    if (hasGUI())
        updateDisplayString();
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
    Enter_Method("receiveModeSetChangeNotification");
    if (signalID == NF_MODESET_CHANGED) {
        modeSet = check_and_cast<Ieee80211ModeSet*>(obj);
        calculateTimingParameters();
    }
}

void Edcaf::updateDisplayString()
{
    std::string displayString(printAccessCategory(ac));
    if (owning)
        displayString += "\n(Channel owner)";
    else if (contention->isContentionInProgress())
        displayString += "\n(Contention in progress)";
    getDisplayString().setTagArg("t", 0, displayString.c_str());
}

} // namespace ieee80211
} // namespace inet
