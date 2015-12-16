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

#include "Tx.h"
#include "IUpperMac.h"
#include "IMacRadioInterface.h"
#include "IRx.h"
#include "IStatistics.h"
#include "Ieee80211Frame_m.h"

namespace inet {
namespace ieee80211 {

Define_Module(Tx);

Tx::~Tx()
{
    cancelAndDelete(endIfsTimer);
    if (frame && !transmitting)
        delete frame;
}

void Tx::initialize()
{
    mac = dynamic_cast<IMacRadioInterface *>(getModuleByPath(par("macModule")));
    upperMac = dynamic_cast<IUpperMac *>(getModuleByPath(par("upperMacModule")));
    rx = dynamic_cast<IRx *>(getModuleByPath(par("rxModule")));
    statistics = check_and_cast<IStatistics*>(getModuleByPath(par("statisticsModule")));
    endIfsTimer = new cMessage("endIFS");

    WATCH(transmitting);
    updateDisplayString();
}

void Tx::transmitFrame(Ieee80211Frame *frame, ITxCallback *txCallback)
{
    transmitFrame(frame, SIMTIME_ZERO, txCallback); //TODO make dedicated version, without the timer
}

void Tx::transmitFrame(Ieee80211Frame *frame, simtime_t ifs, ITxCallback *txCallback)
{
    Enter_Method("transmitFrame(\"%s\")", frame->getName());
    take(frame);
    this->frame = frame;
    this->txCallback = txCallback;

    ASSERT(!endIfsTimer->isScheduled() && !transmitting);    // we are idle
    scheduleAt(simTime() + ifs, endIfsTimer);
    if (hasGUI())
        updateDisplayString();
}

void Tx::radioTransmissionFinished()
{
    Enter_Method_Silent();
    if (transmitting) {
        EV_DETAIL << "Tx: radioTransmissionFinished()\n";
        upperMac->transmissionComplete(txCallback);
        transmitting = false;
        frame = nullptr;
        rx->frameTransmitted(durationField);
        if (hasGUI())
            updateDisplayString();
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
            updateDisplayString();
    }
    else
        ASSERT(false);
}

void Tx::updateDisplayString()
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

