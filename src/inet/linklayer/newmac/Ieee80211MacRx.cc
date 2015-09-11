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

#include "Ieee80211MacRx.h"
#include "IIeee80211MacTx.h"
#include "IIeee80211UpperMac.h"

namespace inet {

namespace ieee80211 {

Ieee80211MacRx::Ieee80211MacRx()
{
}

Ieee80211MacRx::~Ieee80211MacRx()
{
    delete cancelEvent(endNavTimer);
}

void Ieee80211MacRx::initialize()
{
    tx = check_and_cast<IIeee80211MacTx*>(getModuleByPath("^.tx"));  //TODO
    upperMac = check_and_cast<IIeee80211UpperMac*>(getModuleByPath("^.upperMac")); //TODO
    endNavTimer = new cMessage("NAV");
}

void Ieee80211MacRx::handleMessage(cMessage* msg)
{
    if (msg == endNavTimer)  //FIXME should signal to tx!!!! or not????
        EV_INFO << "The radio channel has become free according to the NAV" << std::endl;
    else
        throw cRuntimeError("Unexpected self message");
}

void Ieee80211MacRx::lowerFrameReceived(Ieee80211Frame* frame)
{
    if (!frame)
        throw cRuntimeError("message from physical layer (%s)%s is not a subclass of Ieee80211Frame", frame->getClassName(), frame->getName());
    bool errorFree = isFcsOk(frame);
    tx->lowerFrameReceived(errorFree);
    if (errorFree)
    {
        EV_INFO << "Received message from lower layer: " << frame << endl;
        if (frame->getReceiverAddress() != address)
            setNav(frame->getDuration());
        upperMac->lowerFrameReceived(frame);
    }
    else
    {
        EV_INFO << "Received an erroneous frame. Dropping it." << std::endl;
        delete frame;
    }

}

bool Ieee80211MacRx::isFcsOk(Ieee80211Frame* frame) const
{
    return !frame->hasBitError();
}

bool Ieee80211MacRx::isMediumFree() const
{
    // note: the duration of mode switching (rx-to-tx or tx-to-rx) also counts as busy
    return receptionState == IRadio::RECEPTION_STATE_IDLE && transmissionState == IRadio::TRANSMISSION_STATE_UNDEFINED && !endNavTimer->isScheduled();
}

void Ieee80211MacRx::receptionStateChanged(IRadio::ReceptionState newReceptionState)
{
    receptionState = newReceptionState;  //TODO notification of Tx ????????
}

void Ieee80211MacRx::transmissionStateChanged(IRadio::TransmissionState newTransmissionState)
{
    transmissionState = newTransmissionState; //TODO notification of Tx ????????
}

void Ieee80211MacRx::setNav(simtime_t navInterval)
{
    ASSERT(navInterval >= 0);
    if (endNavTimer->isScheduled())
    {
        simtime_t oldNav = endNavTimer->getArrivalTime() - simTime();
        if (oldNav > navInterval)
            return;
        cancelEvent(endNavTimer);
    }
    if (navInterval > 0)
    {
        EV_INFO << "Setting NAV to " << navInterval << std::endl;
        scheduleAt(simTime() + navInterval, endNavTimer);
    }
    else
        EV_INFO << "Frame duration field is 0" << std::endl; // e.g. Cf-End frame
}

}

} /* namespace inet */

