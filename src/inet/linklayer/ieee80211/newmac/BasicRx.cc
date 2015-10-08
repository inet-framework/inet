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

#include "BasicRx.h"
#include "IContentionTx.h"
#include "IImmediateTx.h"
#include "IUpperMac.h"

namespace inet {
namespace ieee80211 {

Define_Module(BasicRx);

BasicRx::BasicRx()
{
}

BasicRx::~BasicRx()
{
    delete cancelEvent(endNavTimer);
}

void BasicRx::initialize()
{
    upperMac = check_and_cast<IUpperMac *>(getModuleByPath(par("upperMacModule")));
    collectContentionTxModules(getModuleByPath(par("firstContentionTxModule")), contentionTx);
    endNavTimer = new cMessage("NAV");
    recomputeMediumFree();

    WATCH(address);
    WATCH(receptionState);
    WATCH(transmissionState);
    WATCH(mediumFree);
}

void BasicRx::handleMessage(cMessage *msg)
{
    if (msg == endNavTimer) {
        EV_INFO << "The radio channel has become free according to the NAV" << std::endl;
        recomputeMediumFree();
    }
    else
        throw cRuntimeError("Unexpected self message");
}

void BasicRx::lowerFrameReceived(Ieee80211Frame *frame)
{
    Enter_Method("lowerFrameReceived(\"%s\")", frame->getName());
    take(frame);

    bool isFrameOk = isFcsOk(frame);
    if (!isFrameOk)
        for (int i = 0; contentionTx[i]; i++)
            contentionTx[i]->corruptedFrameReceived();

    if (isFrameOk) {
        EV_INFO << "Received message from lower layer: " << frame << endl;
        if (frame->getReceiverAddress() != address)
            setOrExtendNav(frame->getDuration());
        upperMac->lowerFrameReceived(frame);
    }
    else {
        EV_INFO << "Received an erroneous frame. Dropping it." << std::endl;
        delete frame;
    }
}

bool BasicRx::isFcsOk(Ieee80211Frame *frame) const
{
    return !frame->hasBitError();
}

void BasicRx::recomputeMediumFree()
{
    bool oldMediumFree = mediumFree;
    // note: the duration of mode switching (rx-to-tx or tx-to-rx) should also count as busy
    mediumFree = receptionState == IRadio::RECEPTION_STATE_IDLE && transmissionState == IRadio::TRANSMISSION_STATE_UNDEFINED && !endNavTimer->isScheduled();
    if (mediumFree != oldMediumFree) {
        for (int i = 0; contentionTx[i]; i++)
            contentionTx[i]->mediumStateChanged(mediumFree);
    }
}

void BasicRx::receptionStateChanged(IRadio::ReceptionState state)
{
    Enter_Method_Silent();
    receptionState = state;
    recomputeMediumFree();
}

void BasicRx::transmissionStateChanged(IRadio::TransmissionState state)
{
    Enter_Method_Silent();
    transmissionState = state;
    recomputeMediumFree();
}

void BasicRx::setOrExtendNav(simtime_t navInterval)
{
    ASSERT(navInterval >= 0);
    if (navInterval > 0) {
        simtime_t endNav = simTime() + navInterval;
        if (endNavTimer->isScheduled()) {
            simtime_t oldEndNav = endNavTimer->getArrivalTime();
            if (endNav < oldEndNav)
                return;    // never decrease NAV
            cancelEvent(endNavTimer);
        }
        EV_INFO << "Setting NAV to " << navInterval << std::endl;
        scheduleAt(endNav, endNavTimer);
        recomputeMediumFree();
    }
}

} // namespace ieee80211
} // namespace inet

