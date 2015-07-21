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

#include "Ieee80211MacPowerSaveMonitor.h"
#include "inet/common/ModuleAccess.h"

namespace inet {
namespace ieee80211 {

Define_Module(Ieee80211MacPowerSaveMonitor);

void Ieee80211MacPowerSaveMonitor::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
        handleResetMac();
        macsorts = getModuleFromPar<Ieee80211MacMacsorts>(par("macsortsPackage"), this);
        macmib = getModuleFromPar<Ieee80211MacMacmibPackage>(par("macmibPackage"), this);
    }
}

void Ieee80211MacPowerSaveMonitor::processSignal(cMessage *msg)
{
    if (dynamic_cast<Ieee80211MacSignalPsIndicate *>(msg->getControlInfo()))
        handlePsIndicate(dynamic_cast<Ieee80211MacSignalPsIndicate *>(msg->getControlInfo()));
    else if (dynamic_cast<Ieee80211MacSignalStaState *>(msg->getControlInfo()))
        handleStaState(dynamic_cast<Ieee80211MacSignalStaState *>(msg->getControlInfo()));
    else if (dynamic_cast<Ieee80211MacSignalPsInquiry *>(msg->getControlInfo()))
        handlePsInquiry(dynamic_cast<Ieee80211MacSignalPsInquiry *>(msg->getControlInfo()));
    else if (dynamic_cast<Ieee80211MacSignalSsInquiry *>(msg->getControlInfo()))
        handleSsInquiry(dynamic_cast<Ieee80211MacSignalSsInquiry *>(msg->getControlInfo()));
}

void Ieee80211MacPowerSaveMonitor::handleResetMac()
{
    asoc.clear();
    awake.clear();
    asleep.clear();
    state = POWER_SAVE_MONITOR_STATE_MONITOR_IDLE;
}

void Ieee80211MacPowerSaveMonitor::handlePsIndicate(Ieee80211MacSignalPsIndicate* psIndicate)
{
    if (state == POWER_SAVE_MONITOR_STATE_MONITOR_IDLE)
    {
        sta = psIndicate->getSta();
        psm = psIndicate->getPsm();
        if (psm == PsMode_sta_active)
        {
            awake.insert(sta);
            // Send PsChange when sleeping
            // station reports active mode.
            if (asleep.count(sta) == 1)
                emitPsChange(sta, PsMode_sta_active);
            asleep.erase(sta);
        }
        else if (psm == PsMode_power_save)
        {
            awake.erase(sta);
            asleep.insert(sta);
        }
    }
}

void Ieee80211MacPowerSaveMonitor::handleStaState(Ieee80211MacSignalStaState* staState)
{
    if (state == POWER_SAVE_MONITOR_STATE_MONITOR_IDLE)
    {
        sta = staState->getSta();
        sst = staState->getSst();
        if (sst == StationState_asoc)
            asoc.insert(sta);
        else if (sst == StationState_auth_open)
        {
            authOs.insert(sta);
            authKey.erase(sta);
        }
        else if (sst == StationState_auth_key)
        {
            authKey.insert(sta);
            authOs.erase(sta);
        }
        else if (sst == StationState_not_auth) // Note: de_auth is undefined in the standard
        {
            authOs.erase(sta);
            authKey.erase(sta);
            // Deauthenticate of associated
            // station causes disassociate
            // at same time.
            if (asoc.count(sta) == 1)
                asoc.erase(sta);
        }
        else if (sst == StationState_dis_asoc)
            asoc.erase(sta);
        else ;
    }
}

void Ieee80211MacPowerSaveMonitor::handlePsInquiry(Ieee80211MacSignalPsInquiry* psInquiry)
{
    if (state == POWER_SAVE_MONITOR_STATE_MONITOR_IDLE)
    {
        sta = psInquiry->getMacAddress();
        if (!sta.isMulticast())
        {
            if (awake.count(sta) == 1)
            {
                psm = PsMode_sta_active; // TODO: awake?
            }
            else if (asleep.count(sta) == 1)
                psm = PsMode_power_save; // TODO: asleep??
            else
                psm = PsMode_unknown;
        }
        else
            psm = PsMode_unknown;
        emitPsResponse(sta, psm);
    }
}

void Ieee80211MacPowerSaveMonitor::handleSsInquiry(Ieee80211MacSignalSsInquiry* ssInquiry)
{
    if (state == POWER_SAVE_MONITOR_STATE_MONITOR_IDLE)
    {
        sta = ssInquiry->getSta();
        if (sta.isMulticast())
            grpAddr();
        else if (authOs.count(sta) == 1)
        {
            asst = StationState_auth_open;
            function1();
        }
        else if (authKey.count(sta) == 1)
        {
            asst = StationState_auth_key;
            function1();
        }
        else
        {
            asst = StationState_not_auth;
            sst = asst;
        }
        emitSsResponse(sta, sst, asst);
    }
}

void Ieee80211MacPowerSaveMonitor::grpAddr()
{
    sst = StationState_not_auth;
    if (macsorts->getIntraMacRemoteVariables()->isAssoc())
        sst = StationState_asoc;
    else
        sst = StationState_dis_asoc;
    emitSsResponse(sta, sst, asst);
}


void Ieee80211MacPowerSaveMonitor::function1()
{
    if (asoc.count(sta) == 1)
        sst = StationState_asoc;
    else
        sst = asst;
}

void Ieee80211MacPowerSaveMonitor::emitSsResponse(MACAddress sta, StationState sst, StationState asst)
{
    Ieee80211MacSignalSsResponse *signal = new Ieee80211MacSignalSsResponse();
    signal->setSta(sta);
    signal->setSst(sst);
    signal->setAsst(asst);
    cMessage *ssResponse = createSignal("ssResponse", signal);
    send(ssResponse, "sst$o");
}

void Ieee80211MacPowerSaveMonitor::emitPsChange(MACAddress sta, PsMode psMode)
{
    Ieee80211MacSignalPsChange *signal = new Ieee80211MacSignalPsChange();
    signal->setAddr(sta);
    signal->setPsMode(psMode);
    cMessage *psChange = createSignal("psChange", signal);
    send(psChange, "psm$o");
}

void Ieee80211MacPowerSaveMonitor::emitPsResponse(MACAddress sta, PsMode psMode)
{
    Ieee80211MacSignalPsResponse *signal = new Ieee80211MacSignalPsResponse();
    signal->setAdr(sta);
    signal->setPsMode(psMode);
    cMessage *psResponse = createSignal("psResponse", signal);
    send(psResponse, "psm$o");
}

} /* namespace inet */
} /* namespace ieee80211 */
