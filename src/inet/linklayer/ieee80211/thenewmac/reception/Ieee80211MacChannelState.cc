//
// Copyright (C) 2015 OpenSim Ltd.
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
// along with this program; if not, see <http://www.gnu.org/licenses/>.
// 

#include "inet/linklayer/ieee80211/thenewmac/reception/Ieee80211MacChannelState.h"
#include "inet/linklayer/ieee80211/thenewmac/transmission/Ieee80211MacDataPump.h"
#include "inet/common/ModuleAccess.h"

namespace inet {
namespace ieee80211 {

Define_Module(Ieee80211MacChannelState);

void Ieee80211MacChannelState::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
        macsorts = getModuleFromPar<Ieee80211MacMacsorts>(par("macsortsPackage"), this);
        macmib = getModuleFromPar<Ieee80211MacMacmibPackage>(par("macmibPackage"), this);
        tNav = new cMessage("tNav");
        tIfs = new cMessage("tIfs");
        tSlot = new cMessage("tSlot");
    }
    else if (stage == INITSTAGE_LINK_LAYER)
    {
        dSifs = usecToSimtime(macmib->getPhyOperationTable()->getSifsTime());
        dSlot = usecToSimtime(macmib->getPhyOperationTable()->getSlotTime());
        dDifs = dSifs + 2 * dSlot;
//        dIfs = ;
        std::cout << macmib->getPhyOperationTable()->getSlotTime() << endl;
        std::cout << dSlot << endl;
//        Eifs based on the lowest basic rate.
// TODO: aMpduDurationFactor??????
//        dEifs = usecToSimtime(macmib->getPhyOperationTable()->getSifsTime() + calcDur(1 /*import(mBrates))*/, stuff()))
        dIfs = dEifs;
        // TODO: hack
        dIfs = simtime_t(50E-6);
        handleResetMac();
    }
}

void Ieee80211MacChannelState::handleMessage(cMessage* msg)
{
    EV_DEBUG << "state = " << state << endl;
    if (msg->isSelfMessage())
    {
        if (msg == tNav)
            handleTnav();
        else if (msg == tIfs)
            handleTifs();
        else if (msg == tSlot)
            handleTslot();
        else
            throw cRuntimeError("Unknown self message");
    }
    else
    {
        if (dynamic_cast<Ieee80211MacSignalPhyCcaIndication*>(msg->getControlInfo()))
            handlePhyCcaIndication(dynamic_cast<Ieee80211MacSignalPhyCcaIndication*>(msg->getControlInfo()));
        else if (dynamic_cast<Ieee80211MacSignalSetNav*>(msg->getControlInfo()))
            handleSetNav(dynamic_cast<Ieee80211MacSignalSetNav*>(msg->getControlInfo()));
        else if (dynamic_cast<Ieee80211MacSignalChangeNav*>(msg->getControlInfo()))
            handleChangeNav(dynamic_cast<Ieee80211MacSignalChangeNav*>(msg->getControlInfo()));
        else if (dynamic_cast<Ieee80211MacSignalUseEifs*>(msg->getControlInfo()))
            handleUseEifs(dynamic_cast<Ieee80211MacSignalUseEifs*>(msg->getControlInfo()));
        else if (dynamic_cast<Ieee80211MacSignalUseDifs*>(msg->getControlInfo()))
            handleUseDifs(dynamic_cast<Ieee80211MacSignalUseDifs*>(msg->getControlInfo()));
        else if (dynamic_cast<Ieee80211MacSignalClearNav*>(msg->getControlInfo()))
            handleClearNav(dynamic_cast<Ieee80211MacSignalClearNav*>(msg->getControlInfo()));
        else if (dynamic_cast<Ieee80211MacSignalRtsTimeout*>(msg->getControlInfo()))
            handleRtsTimeout();
        else
            throw cRuntimeError("Unknown signal");
    }
}

void Ieee80211MacChannelState::handleResetMac()
{
    dIfs = dEifs;
//    Assume channel busy until Phy
//    indicates idle. tNavEnd is =0
//    until first rx that sets Nav.
    cs = CcaStatus_busy;
    tNavEnd = SIMTIME_ZERO;
    cancelEvent(tNav);
    emitPhyCcarstRequest();
    macsorts->getIntraMacRemoteVariables()->setNavEnd(tNavEnd);
    curSrc = NavSrc_nosrc;
    emitBusy();
    state = CHANNEL_STATE_STATE_CS_NO_NAV;
}

void Ieee80211MacChannelState::emitIdle()
{
    Ieee80211MacSignalIdle *signal = new Ieee80211MacSignalIdle();
    cMessage *idle = createSignal("Idle", signal);
    send(idle, "toTx");
}

void Ieee80211MacChannelState::emitSlot()
{
    Ieee80211MacSignalSlot *signal = new Ieee80211MacSignalSlot();
    cMessage *slot = createSignal("Slot", signal);
    send(slot, "toTx");
}

void Ieee80211MacChannelState::emitBusy()
{
    Ieee80211MacSignalBusy *signal = new Ieee80211MacSignalBusy();
    cMessage *busy = createSignal("Busy", signal);
    send(busy, "toTx");
}

void Ieee80211MacChannelState::handleTifs()
{
    if (state == CHANNEL_STATE_STATE_WAIT_IFS)
    {
        emitIdle();
        scheduleAt(simTime() + dSlot, tSlot);
        state = CHANNEL_STATE_STATE_NO_CS_NO_NAV;
    }
}

void Ieee80211MacChannelState::handlePhyCcaIndication(Ieee80211MacSignalPhyCcaIndication* phyCcaIndication)
{
    if (state == CHANNEL_STATE_STATE_WAIT_IFS)
    {
        cs = phyCcaIndication->getStatus();
        if (cs == CcaStatus_busy)
            state = CHANNEL_STATE_STATE_CS_NO_NAV;
    }
    else if (state == CHANNEL_STATE_STATE_CS_NO_NAV)
    {
        cs = phyCcaIndication->getStatus();
        if (cs == CcaStatus_idle)
        {
            std::cout << dIfs << endl;
            scheduleAt(simTime() + dIfs, tIfs);
            state = CHANNEL_STATE_STATE_WAIT_IFS;
        }
    }
    else if (state == CHANNEL_STATE_STATE_NO_CS_NO_NAV)
    {
        cs = phyCcaIndication->getStatus();
        if (cs == CcaStatus_busy)
        {
            emitBusy();
            state = CHANNEL_STATE_STATE_CS_NO_NAV;
        }
    }
    else if (state == CHANNEL_STATE_STATE_NO_CS_NAV)
    {
        cs = phyCcaIndication->getStatus();
        if (cs == CcaStatus_busy)
            state = CHANNEL_STATE_STATE_CS_NAV;
    }
    else if (state == CHANNEL_STATE_STATE_CS_NAV)
    {
        if (cs == CcaStatus_idle)
            state = CHANNEL_STATE_STATE_NO_CS_NAV;
    }
}

void Ieee80211MacChannelState::handleSetNav(Ieee80211MacSignalSetNav *setNav)
{
    if (state == CHANNEL_STATE_STATE_CS_NO_NAV || state == CHANNEL_STATE_STATE_WAIT_IFS)
    {
        this->tRef = setNav->getTime();
        this->dNav = setNav->getDuration();
        this->curSrc = setNav->getNavSrc();
        tNavEnd = tRef + dNav;
        scheduleAt(tNavEnd, tNav);
        macsorts->getIntraMacRemoteVariables()->setNavEnd(tNavEnd);
        state = CHANNEL_STATE_STATE_CS_NAV;
    }
    else if (state == CHANNEL_STATE_STATE_NO_CS_NO_NAV)
    {
        this->tRef = setNav->getTime();
        this->dNav = setNav->getDuration();
        this->curSrc = setNav->getNavSrc();
        tNavEnd = tRef + dNav;
        scheduleAt(tNavEnd, tNav);
        emitBusy();
        macsorts->getIntraMacRemoteVariables()->setNavEnd(tNavEnd);
        state = CHANNEL_STATE_STATE_NO_CS_NAV;
    }
    else if (state == CHANNEL_STATE_STATE_CS_NAV || state == CHANNEL_STATE_STATE_NO_CS_NAV)
    {
        this->tRef = setNav->getTime();
        this->dNav = setNav->getDuration();
        this->curSrc = setNav->getNavSrc();
        tNew = tRef + dNav;
        if (tNew > tNavEnd)
        {
            tNavEnd = tNew;
            curSrc = newSrc;
        }
        scheduleAt(tNavEnd, tNav);
        macsorts->getIntraMacRemoteVariables()->setNavEnd(tNavEnd);
    }
}

void Ieee80211MacChannelState::handleTslot()
{
    if (state == CHANNEL_STATE_STATE_NO_CS_NO_NAV)
    {
        emitSlot();
        scheduleAt(simTime() + dSlot, tSlot);
    }
}

void Ieee80211MacChannelState::handleTnav()
{
    if (state == CHANNEL_STATE_STATE_NO_CS_NAV)
    {
        emitPhyCcarstRequest();
        curSrc = NavSrc_nosrc;
        scheduleAt(simTime() + dIfs, tIfs);
        state = CHANNEL_STATE_STATE_WAIT_IFS;
    }
    else if (state == CHANNEL_STATE_STATE_CS_NAV)
    {
        emitPhyCcarstRequest();
        curSrc = NavSrc_nosrc;
        scheduleAt(simTime() + dIfs, tIfs);
        state = CHANNEL_STATE_STATE_CS_NO_NAV;
    }
}

void Ieee80211MacChannelState::handleUseEifs(Ieee80211MacSignalUseEifs *useEifs)
{
    tRxEnd = useEifs->getTime();
    dIfs = dEifs - dRxTx;
    scheduleAt(tRxEnd + dIfs, tIfs);
}

void Ieee80211MacChannelState::handleUseDifs(Ieee80211MacSignalUseDifs *useDifs)
{
    tRxEnd = useDifs->getTime();
    dIfs = dDifs - dRxTx;
    scheduleAt(tRxEnd + dIfs, tIfs);
}

void Ieee80211MacChannelState::handleChangeNav(Ieee80211MacSignalChangeNav* changeNav)
{
    this->tRef = changeNav->getTime();
    this->dNav = changeNav->getDuration();
    this->curSrc = changeNav->getNavSrc();
    if (newSrc == NavSrc_cswitch)
    {
        dIfs = dEifs - dRxTx;
        scheduleAt(simTime(), tNav);
        tNavEnd = SIMTIME_ZERO;
        curSrc = NavSrc_nosrc;
        macsorts->getIntraMacRemoteVariables()->setNavEnd(tNavEnd);
    }
    else
    {
        tNew = tRef + dNav;
//        Nav is cleared by setting Tnav
//        to now. This causes immediate
//        Tnav signal to enable exit from
//        noCs_Nav or Cs_Nav state.
        if (tNew > tNavEnd)
        {
            tNavEnd = tNew;
            curSrc = newSrc;
        }
        scheduleAt(tNavEnd, tNav);
        macsorts->getIntraMacRemoteVariables()->setNavEnd(tNavEnd);
    }
}

void Ieee80211MacChannelState::handleRtsTimeout()
{
    if (state == CHANNEL_STATE_STATE_CS_NAV || state == CHANNEL_STATE_STATE_NO_CS_NAV)
    {
        if (curSrc == NavSrc_rts)
        {
            tNavEnd = simTime();
            curSrc = NavSrc_nosrc;
            scheduleAt(tNavEnd, tNav);
            macsorts->getIntraMacRemoteVariables()->setNavEnd(tNavEnd);
        }
    }
}

void Ieee80211MacChannelState::handleClearNav(Ieee80211MacSignalClearNav *clearNav)
{
    if (state == CHANNEL_STATE_STATE_CS_NAV || state == CHANNEL_STATE_STATE_NO_CS_NAV)
    {
        newSrc = clearNav->getNavSrc();
        tNavEnd = simTime();
        curSrc = NavSrc_nosrc;
        scheduleAt(tNavEnd, tNav);
        macsorts->getIntraMacRemoteVariables()->setNavEnd(tNavEnd);
    }
}

void Ieee80211MacChannelState::emitPhyCcarstRequest()
{
//    cMessage *phyCcarstRequest = new cMessage("PhyCcarstRequest");
//    Ieee80211MacSignalPhyCcarstRequest *signal = new Ieee80211MacSignalPhyCcarstRequest();
//    phyCcarstRequest->setControlInfo(signal);
    // TODO
}

} /* namespace ieee80211 */
} /* namespace inet */

