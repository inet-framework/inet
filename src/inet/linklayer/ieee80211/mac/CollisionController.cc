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

#include "CollisionController.h"

namespace inet {
namespace ieee80211 {

Define_Module(CollisionController);

CollisionController::CollisionController()
{
    for (int i = 0; i < MAX_NUM_TX; i++)
        timer[i] = nullptr;
}

CollisionController::~CollisionController()
{
    for (int i = 0; i < MAX_NUM_TX; i++)
        cancelAndDelete(timer[i]);
}

void CollisionController::scheduleTransmissionRequest(int txIndex, simtime_t txStartTime, ICallback *cb)
{
    Enter_Method("scheduleTransmissionRequest(%d)", txIndex);
    ASSERT(txIndex >=0 && txIndex < MAX_NUM_TX);
    ASSERT(txStartTime > timeLastProcessed); // if equal then it's too late, that round was already done and notified (check timer's scheduling priority if that happens!)

    if (txCount <= txIndex)
        txCount = txIndex+1;

    if (timer[txIndex] == nullptr) {
        char name[16];
        sprintf(name, "txStart-%d", txIndex);
        timer[txIndex] = new cMessage(name);
        timer[txIndex]->setSchedulingPriority(1000); // low priority, i.e. processed later than most events for the same time
    }

    callback[txIndex] = cb;

    ASSERT(!timer[txIndex]->isScheduled());
    scheduleAt(txStartTime, timer[txIndex]);
}

void CollisionController::cancelTransmissionRequest(int txIndex)
{
    Enter_Method("cancelTransmissionRequest(%d)", txIndex);
    ASSERT(txIndex >=0 && txIndex < txCount && timer[txIndex] != nullptr);
    cancelEvent(timer[txIndex]);
}

void CollisionController::handleMessage(cMessage *msg)
{
    // from the ones scheduled for the current simulation time: grant transmission to the
    // highest priority one (largest txIndex), and signal internal collision to the others
    simtime_t now = simTime();
    bool granted = false;
    for (int i = txCount-1; i >= 0; i--) {
        if (timer[i] == msg || (timer[i] && timer[i]->isScheduled() && timer[i]->getArrivalTime() == now)) {
            cancelEvent(timer[i]);
            if (!granted) {
                callback[i]->transmissionGranted(i);
                granted = true;
            }
            else {
                callback[i]->internalCollision(i);
            }
        }
    }
    timeLastProcessed = now;
}

} // namespace ieee80211
} // namespace inet
