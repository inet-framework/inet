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

#include "Ieee80211MacReception.h"
#include "Ieee80211MacTransmission.h"
#include "Ieee80211NewMac.h"
#include "Ieee80211UpperMac.h"
#include "Ieee80211MacContext.h"

namespace inet {

namespace ieee80211 {

bool Ieee80211MacReception::isFcsOk(Ieee80211Frame* frame) const
{
    return !frame->hasBitError();
}

Ieee80211MacReception::Ieee80211MacReception(Ieee80211NewMac* mac) : Ieee80211MacPlugin(mac)
{
    nav = new cMessage("NAV");
}


void Ieee80211MacReception::handleMessage(cMessage* msg)
{
    if (msg == nav)
        EV_INFO << "The radio channel has become free according to the NAV" << std::endl;
    else
        throw cRuntimeError("Unexpected self message");
}

void Ieee80211MacReception::handleLowerFrame(Ieee80211Frame* frame)
{
    if (!frame)
        throw cRuntimeError("message from physical layer (%s)%s is not a subclass of Ieee80211Frame", frame->getClassName(), frame->getName());
    bool errorFree = isFcsOk(frame);
    getTransmission()->lowerFrameReceived(errorFree);
    if (errorFree)
    {
        EV_INFO << "Received message from lower layer: " << frame << endl;
        if (!context->isForUs(frame))
            setNav(frame->getDuration());
        Ieee80211TwoAddressFrame *twoAddressFrame = dynamic_cast<Ieee80211TwoAddressFrame *>(frame);
        ASSERT(!twoAddressFrame || twoAddressFrame->getTransmitterAddress() != context->getAddress());
        getUpperMac()->lowerFrameReceived(frame);
    }
    else
    {
        EV_INFO << "Received an erroneous frame. Dropping it." << std::endl;
        delete frame;
    }

}

bool Ieee80211MacReception::isMediumFree() const
{
    return receptionState == IRadio::RECEPTION_STATE_IDLE && transmissionState != IRadio::TRANSMISSION_STATE_TRANSMITTING && !nav->isScheduled();
}

void Ieee80211MacReception::receptionStateChanged(IRadio::ReceptionState newReceptionState)
{
    receptionState = newReceptionState;
}

void Ieee80211MacReception::transmissionStateChanged(IRadio::TransmissionState newTransmissionState)
{
    transmissionState = newTransmissionState;
}

void Ieee80211MacReception::setNav(simtime_t navInterval)
{
    ASSERT(navInterval >= 0);
    if (nav->isScheduled())
    {
        simtime_t oldNav = nav->getArrivalTime() - simTime();
        if (oldNav > navInterval)
            return;
        cancelEvent(nav);
    }
    if (navInterval > 0)
    {
        EV_INFO << "Setting NAV to " << navInterval << std::endl;
        scheduleAt(simTime() + navInterval, nav);
    }
    else
        EV_INFO << "Frame duration field is 0" << std::endl; // e.g. Cf-End frame
}

Ieee80211MacReception::~Ieee80211MacReception()
{
    cancelEvent(nav);
    delete nav;
}

}

} /* namespace inet */

