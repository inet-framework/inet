//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "Ieee80211MacImmediateTx.h"
#include "Ieee80211NewMac.h"
#include "inet/common/FSMA.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

namespace inet {

namespace ieee80211 {

Ieee80211MacImmediateTx::Ieee80211MacImmediateTx(Ieee80211NewMac *mac) : Ieee80211MacPlugin(mac)
{
    endIfsTimer = new cMessage("endIFS");
}

Ieee80211MacImmediateTx::~Ieee80211MacImmediateTx()
{
    //TODO cancelAndDelete(endIfsTimer);
    delete frame;
}

void Ieee80211MacImmediateTx::transmitImmediateFrame(Ieee80211Frame* frame, simtime_t ifs, IIeee80211MacImmediateTx::ICallback *completionCallback)
{
    ASSERT(!endIfsTimer->isScheduled() && !transmitting); // we are idle
    scheduleAt(simTime() + ifs, endIfsTimer);
    this->frame = frame;
    this->completionCallback = completionCallback;
}

void Ieee80211MacImmediateTx::transmissionFinished()
{
    if (transmitting) {
        completionCallback->immediateTransmissionComplete();
        transmitting = false;
        frame = nullptr;
    }
}

void Ieee80211MacImmediateTx::handleMessage(cMessage *msg)
{
    if (msg == endIfsTimer) {
        transmitting = true;
        mac->sendFrame(frame);
    }
    else
        ASSERT(false);
}

}

} //namespace

