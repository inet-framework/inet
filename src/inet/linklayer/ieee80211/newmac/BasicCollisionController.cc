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

#include "BasicCollisionController.h"

namespace inet {
namespace ieee80211 {

Define_Module(BasicCollisionController);

simtime_t BasicCollisionController::MAX_TIME;

BasicCollisionController::BasicCollisionController()
{
    MAX_TIME = SimTime::getMaxTime();
    for (int i = 0; i < MAX_NUM_TX; i++)
        txTime[i] = MAX_TIME;
    timer = new cMessage("nextTx");
    timer->setSchedulingPriority(1000); // low priority, i.e. processed later than most events for the same time
}

BasicCollisionController::~BasicCollisionController()
{
    cancelAndDelete(timer);
}

void BasicCollisionController::scheduleTransmissionRequest(int txIndex, simtime_t txStartTime, ICallback *cb)
{
    Enter_Method("scheduleTransmissionRequest(%d)", txIndex);
    ASSERT(txIndex >=0 && txIndex < MAX_NUM_TX);
    ASSERT(txTime[txIndex] == MAX_TIME);  // not yet scheduled
    ASSERT(txStartTime > timeLastProcessed); // if equal then it's too late, that round was already done and notified (check timer's scheduling priority if that happens!)

    if (maxTxIndex < txIndex)
        maxTxIndex = txIndex;

    // store request
    txTime[txIndex] = txStartTime;
    callback[txIndex] = cb;

    // reschedule timer if needed
    bool isScheduled = timer->isScheduled();
    if (!isScheduled || txStartTime < timer->getArrivalTime()) {
        if (isScheduled)
            cancelEvent(timer);
        scheduleAt(txStartTime, timer);
    }
}

void BasicCollisionController::cancelTransmissionRequest(int txIndex)
{
    Enter_Method("cancelTransmissionRequest(%d)", txIndex);
    ASSERT(txIndex >=0 && txIndex < MAX_NUM_TX);
    txTime[txIndex] = MAX_TIME;
    reschedule();
}

void BasicCollisionController::handleMessage(cMessage *msg)
{
    ASSERT(msg == timer);

    // from the ones with the current time as timestamp: grant transmission to the
    // highest priority one (largest txIndex), and signal internal collision to the others
    simtime_t now = simTime();
    bool granted = false;
    for (int i = maxTxIndex; i >= 0; i--) {
        if (txTime[i] == now) {
            txTime[i] = MAX_TIME;
            if (!granted) {
                callback[i]->transmissionGranted(i);
                granted = true;
            }
            else {
                callback[i]->internalCollision(i);
            }
        }
    }

    // reschedule next event
    reschedule();

    timeLastProcessed = now;
}

void BasicCollisionController::reschedule()
{
    // choose smallest time
    simtime_t nextTxTime = MAX_TIME;
    for (int i = 0; i < maxTxIndex; i++)
        if (txTime[i] < nextTxTime)
            nextTxTime = txTime[i];

    // reschedule timer for it
    if (!timer->isScheduled()) {
        if (nextTxTime != MAX_TIME)
            scheduleAt(nextTxTime, timer);
    }
    else {
        if (timer->getArrivalTime() != nextTxTime) {
            cancelEvent(timer);
            if (nextTxTime != MAX_TIME)
                scheduleAt(nextTxTime, timer);
        }
    }
}

} // namespace ieee80211
} // namespace inet
