//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//

#include "inet/mobility/base/MovingMobilityBase.h"

namespace inet {

MovingMobilityBase::MovingMobilityBase() :
    moveTimer(nullptr),
    updateInterval(0),
    stationary(false),
    lastVelocity(Coord::ZERO),
    lastUpdate(0),
    nextChange(-1),
    faceForward(false)
{
}

MovingMobilityBase::~MovingMobilityBase()
{
    cancelAndDelete(moveTimer);
}

void MovingMobilityBase::initialize(int stage)
{
    MobilityBase::initialize(stage);
    EV_TRACE << "initializing MovingMobilityBase stage " << stage << endl;
    if (stage == INITSTAGE_LOCAL) {
        moveTimer = new cMessage("move");
        updateInterval = par("updateInterval");
        faceForward = par("faceForward");
    }
    else if (stage == INITSTAGE_SINGLE_MOBILITY) {
        scheduleUpdate();
    }
}

void MovingMobilityBase::initializeMobilityData()
{
    MobilityBase::initializeMobilityData();
    lastUpdate = simTime();
}

void MovingMobilityBase::moveAndUpdate()
{
    simtime_t now = simTime();
    if (nextChange == now || lastUpdate != now) {
        move();
        orient();
        lastUpdate = simTime();
        emitMobilityStateChangedSignal();
    }
}

Quaternion MovingMobilityBase::getOrientOfVelocity(Coord direction) const
{
    direction.normalize();
    auto alpha = rad(atan2(direction.y, direction.x));
    auto beta = rad(-asin(direction.z));
    auto gamma = rad(0.0);
    return Quaternion(EulerAngles(alpha, beta, gamma));
}

void MovingMobilityBase::orient()
{
    if (faceForward && (lastVelocity != Coord::ZERO)) {
        // determine orientation based on direction
        lastOrientation = getOrientOfVelocity(lastVelocity);
    }
}

void MovingMobilityBase::handleSelfMessage(cMessage *message)
{
    if (message == moveTimer) {
        moveAndUpdate();
        if (simTime() == nextChange)
            changeMobilityData();
        scheduleUpdate();
    }
    else
        throw cRuntimeError("Unknown self message %s", message->getClassAndFullName().c_str());
}

void MovingMobilityBase::scheduleUpdate()
{
    cancelEvent(moveTimer);
    if (!stationary && updateInterval != 0) {
        // periodic update is needed
        simtime_t nextUpdate = simTime() + updateInterval;
        if (nextChange != -1 && nextChange < nextUpdate)
            // next change happens earlier than next update
            scheduleAt(nextChange, moveTimer);
        else
            // next update happens earlier than next change or there is no change at all
            scheduleAt(nextUpdate, moveTimer);
    }
    else if (nextChange != -1)
        // no periodic update is needed
        scheduleAt(nextChange, moveTimer);
}

const Coord& MovingMobilityBase::getCurrentPosition()
{
    moveAndUpdate();
    return lastPosition;
}

const Coord& MovingMobilityBase::getCurrentVelocity()
{
    moveAndUpdate();
    return lastVelocity;
}

const Coord& MovingMobilityBase::getCurrentAcceleration()
{
    return Coord::ZERO;
}

const Quaternion& MovingMobilityBase::getCurrentAngularPosition()
{
    moveAndUpdate();
    return lastOrientation;
}

const Quaternion& MovingMobilityBase::getCurrentAngularVelocity()
{
    moveAndUpdate();
    return lastAngularVelocity;
}

const Quaternion& MovingMobilityBase::getCurrentAngularAcceleration()
{
    return Quaternion::IDENTITY;
}

} // namespace inet

