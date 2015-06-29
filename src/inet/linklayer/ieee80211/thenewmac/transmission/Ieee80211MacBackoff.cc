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

#include "Ieee80211MacBackoff.h"
#include "inet/common/ModuleAccess.h"

namespace inet {
namespace ieee80211 {

Define_Module(Ieee80211MacBackoff);

void Ieee80211MacBackoff::handleMessage(cMessage* msg)
{
    if (msg->isSelfMessage())
    {
        if (strcmp("ResetMac",msg->getName()) == 0)
            handleResetMac();
        else
            throw cRuntimeError("Unknown self message");
    }
    else
    {
        if (dynamic_cast<Ieee80211MacSignalBackoff *>(msg->getControlInfo()))
            handleBackoff(dynamic_cast<Ieee80211MacSignalBackoff *>(msg->getControlInfo()));
        else if (dynamic_cast<Ieee80211MacSignalIdle *>(msg->getControlInfo()))
            handleIdle();
        else if (dynamic_cast<Ieee80211MacSignalSlot *>(msg->getControlInfo()))
            handleSlot();
        else if (dynamic_cast<Ieee80211MacSignalBusy *>(msg->getControlInfo()))
            handleBusy();
        else if (dynamic_cast<Ieee80211MacSignalCancel *>(msg->getControlInfo())) // sender must use scheduling priority
            handleCancel();
        else
            throw cRuntimeError("Unknown input signal");
    }
}

void Ieee80211MacBackoff::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
        macsorts = getModuleFromPar<Ieee80211MacMacsorts>(par("macsortsPackage"), this);
    }
    if (stage == INITSTAGE_LINK_LAYER)
    {
    }
}

void Ieee80211MacBackoff::handleResetMac()
{
    // In all state
    done();
}

void Ieee80211MacBackoff::handleBackoff(Ieee80211MacSignalBackoff *backoff)
{
    // cw is contention window, cnt is slot count from
    // previous BkDone. If cnt<0, a new random count
    // is generated.
    cw = backoff->getCw();
    cnt = backoff->getCnt();
//    source = backoff->getArrivalGate(); // Save PId from request to use as addr of Done.
    macsorts->getIntraMacRemoteVariables()->setBkIp(true);
    // Choose random backoff count if cnt = -1.
    // Resume with count from cancelled backoff if cnt>=0.
    slotCnt = cnt < 0 ? intuniform(0, cw) : cnt;
    // At start assume that the WM is busy until receiving a signal
    // which indicates the WM is idle.
    state = BACKOFF_STATE_CHANNEL_BUSY;
}

void Ieee80211MacBackoff::handleIdle()
{
    // Transitions to Channel_Idle also align the
    // Backoff signal arrival time to slot boundary (M2) timing
    if (state  == BACKOFF_STATE_CHANNEL_BUSY)
    {
        state = BACKOFF_STATE_CHANNEL_IDLE;
        if (slotCnt == 0)
        {
            cnt == 0 ? emitBkDone(-2) : emitBkDone(-1);
            done();
        }
    }
    // Idle signal not sent if WM free. This consumes any
    // Idles still on input queue.
}

void Ieee80211MacBackoff::handleBusy()
{
    if (state == BACKOFF_STATE_CHANNEL_BUSY)
        cnt = 1;
    else if (state == BACKOFF_STATE_CHANNEL_IDLE)
        // Go back and wait for WM to become idle.
        state = BACKOFF_STATE_CHANNEL_BUSY;
}

void Ieee80211MacBackoff::handleSlot()
{
    // Slot only sent when WM idle. This is for the case where WM idle at arrival of Backoff signal.
    if (state == BACKOFF_STATE_CHANNEL_IDLE)
        // Decrement count for each slot when WM idle.
        slotCnt--;
    // else ignore
}

void Ieee80211MacBackoff::handleCancel() // Note: priority signal
{
    if (state == BACKOFF_STATE_CHANNEL_BUSY || state == BACKOFF_STATE_CHANNEL_IDLE)
    {
        emitBkDone(slotCnt);
        done();
    }
    // else ignore
}

void Ieee80211MacBackoff::done()
{
    macsorts->getIntraMacRemoteVariables()->setBkIp(false);
    state = BACKOFF_STATE_NO_BACKOFF;
}

void Ieee80211MacBackoff::emitBkDone(int slotCount)
{
    cMessage *bkDone = new cMessage("bkDone");
    Ieee80211MacSignalBkDone *signal = new Ieee80211MacSignalBkDone();
    signal->setSlotCount(slotCount);
    bkDone->setControlInfo(signal);
    send(bkDone, source);
}

} /* namespace inet */
} /* namespace ieee80211 */

