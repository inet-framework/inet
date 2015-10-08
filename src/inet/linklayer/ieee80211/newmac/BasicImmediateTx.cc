//
// Copyright (C) 2015 Andras Varga
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
// Author: Andras Varga
//

#include "BasicImmediateTx.h"
#include "IUpperMac.h"
#include "IMacRadioInterface.h"
#include "inet/common/FSMA.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

namespace inet {
namespace ieee80211 {

Define_Module(BasicImmediateTx);

BasicImmediateTx::~BasicImmediateTx()
{
    cancelAndDelete(endIfsTimer);
    delete frame;
}

void BasicImmediateTx::initialize()
{
    mac = dynamic_cast<IMacRadioInterface*>(getModuleByPath("^"));
    upperMac = dynamic_cast<IUpperMac*>(getModuleByPath("^.upperMac"));
    endIfsTimer = new cMessage("endIFS");
}

void BasicImmediateTx::transmitImmediateFrame(Ieee80211Frame* frame, simtime_t ifs, ITxCallback *completionCallback)
{
    EV_DETAIL << "BasicImmediateTx: transmitImmediateFrame " << frame->getName() << endl;
    ASSERT(!endIfsTimer->isScheduled() && !transmitting); // we are idle
    scheduleAt(simTime() + ifs, endIfsTimer);
    this->frame = frame;
    this->completionCallback = completionCallback;
}

void BasicImmediateTx::radioTransmissionFinished()
{
    Enter_Method("radioTransmissionFinished()");
    if (transmitting) {
        EV_DETAIL << "BasicImmediateTx: radioTransmissionFinished()\n";
        upperMac->transmissionComplete(completionCallback, -1);
        transmitting = false;
        frame = nullptr;
    }
}

void BasicImmediateTx::handleMessage(cMessage *msg)
{
    if (msg == endIfsTimer) {
        EV_DETAIL << "BasicImmediateTx: endIfsTimer expired\n";
        transmitting = true;
        mac->sendFrame(frame);
    }
    else
        ASSERT(false);
}

} // namespace ieee80211
} // namespace inet

