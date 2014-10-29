//
// Copyright (C) 2006-2012 Christoph Sommer <christoph.sommer@uibk.ac.at>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#ifdef WITH_TRACI

#include <limits>
#include <iostream>
#include <sstream>

#include "inet/common/INETMath.h"    // for M_PI
#include "inet/mobility/group/TraCIMobility.h"

namespace inet {

Define_Module(TraCIMobility);

namespace {
const double MY_INFINITY = (std::numeric_limits<double>::has_infinity ? std::numeric_limits<double>::infinity() : std::numeric_limits<double>::max());

double roadIdAsDouble(std::string road_id)
{
    std::istringstream iss(road_id);
    double d;
    if (!(iss >> d))
        return MY_INFINITY;
    return d;
}
} // namespace {

void TraCIMobility::Statistics::initialize()
{
    firstRoadNumber = MY_INFINITY;
    startTime = simTime();
    totalTime = 0;
    stopTime = 0;
    minSpeed = MY_INFINITY;
    maxSpeed = -MY_INFINITY;
    totalDistance = 0;
    totalCO2Emission = 0;
}

void TraCIMobility::Statistics::watch(cSimpleModule&)
{
    WATCH(totalTime);
    WATCH(minSpeed);
    WATCH(maxSpeed);
    WATCH(totalDistance);
}

void TraCIMobility::Statistics::recordScalars(cSimpleModule& module)
{
    if (firstRoadNumber != MY_INFINITY)
        module.recordScalar("firstRoadNumber", firstRoadNumber);
    module.recordScalar("startTime", startTime);
    module.recordScalar("totalTime", totalTime);
    module.recordScalar("stopTime", stopTime);
    if (minSpeed != MY_INFINITY)
        module.recordScalar("minSpeed", minSpeed);
    if (maxSpeed != -MY_INFINITY)
        module.recordScalar("maxSpeed", maxSpeed);
    module.recordScalar("totalDistance", totalDistance);
    module.recordScalar("totalCO2Emission", totalCO2Emission);
}

void TraCIMobility::initialize(int stage)
{
    //TODO why call the base::initialize() at the end?

    if (stage == INITSTAGE_LOCAL) {
        accidentCount = par("accidentCount");

        currentPosXVec.setName("posx");
        currentPosYVec.setName("posy");
        currentSpeedVec.setName("speed");
        currentAccelerationVec.setName("acceleration");
        currentCO2EmissionVec.setName("co2emission");

        statistics.initialize();
        statistics.watch(*this);

        WATCH(road_id);
        WATCH(speed);
        WATCH(angle);
        WATCH(lastPosition.x);
        WATCH(lastPosition.y);

        startAccidentMsg = 0;
        stopAccidentMsg = 0;
        manager = 0;
        last_speed = -1;

        if (accidentCount > 0) {
            simtime_t accidentStart = par("accidentStart");
            startAccidentMsg = new cMessage("scheduledAccident");
            stopAccidentMsg = new cMessage("scheduledAccidentResolved");
            scheduleAt(simTime() + accidentStart, startAccidentMsg);
        }

        if (ev.isGUI())
            updateDisplayString();
    }

    MobilityBase::initialize(stage);
}

void TraCIMobility::setInitialPosition()
{
    ASSERT(isPreInitialized);
    isPreInitialized = false;
}

void TraCIMobility::finish()
{
    statistics.stopTime = simTime();

    statistics.recordScalars(*this);

    cancelAndDelete(startAccidentMsg);
    cancelAndDelete(stopAccidentMsg);

    isPreInitialized = false;
}

void TraCIMobility::handleSelfMessage(cMessage *msg)
{
    if (msg == startAccidentMsg) {
        commandSetSpeed(0);
        simtime_t accidentDuration = par("accidentDuration");
        scheduleAt(simTime() + accidentDuration, stopAccidentMsg);
        accidentCount--;
    }
    else if (msg == stopAccidentMsg) {
        commandSetSpeed(-1);
        if (accidentCount > 0) {
            simtime_t accidentInterval = par("accidentInterval");
            scheduleAt(simTime() + accidentInterval, startAccidentMsg);
        }
    }
}

void TraCIMobility::preInitialize(std::string external_id, const Coord& position, std::string road_id, double speed, double angle)
{
    this->external_id = external_id;
    this->lastUpdate = 0;
    nextPos = position;
    lastPosition = position;
    this->road_id = road_id;
    this->speed = speed;
    this->angle = angle;

    isPreInitialized = true;
}

void TraCIMobility::nextPosition(const Coord& position, std::string road_id, double speed, double angle, TraCIScenarioManager::VehicleSignal signals)
{
    EV_DEBUG << "next position = " << position << " " << road_id << " " << speed << " " << angle << std::endl;
    isPreInitialized = false;
    nextPos = position;
    this->road_id = road_id;
    this->speed = speed;
    this->angle = angle;
    this->signals = signals;
    move();
}

void TraCIMobility::move()
{
    // ensure we're not called twice in one time step
    ASSERT(lastUpdate != simTime());

    // keep statistics (for current step)
    currentPosXVec.record(lastPosition.x);
    currentPosYVec.record(lastPosition.y);

    // keep statistics (relative to last step)
    if (statistics.startTime != simTime()) {
        simtime_t updateInterval = simTime() - this->lastUpdate;
        this->lastUpdate = simTime();

        double distance = lastPosition.distance(nextPos);
        statistics.totalDistance += distance;
        statistics.totalTime += updateInterval;
        if (speed != -1) {
            statistics.minSpeed = std::min(statistics.minSpeed, speed);
            statistics.maxSpeed = std::max(statistics.maxSpeed, speed);
            currentSpeedVec.record(speed);
            if (last_speed != -1) {
                double acceleration = (speed - last_speed) / updateInterval;
                double co2emission = calculateCO2emission(speed, acceleration);
                currentAccelerationVec.record(acceleration);
                currentCO2EmissionVec.record(co2emission);
                statistics.totalCO2Emission += co2emission * updateInterval.dbl();
            }
            last_speed = speed;
        }
        else {
            last_speed = -1;
            speed = -1;
        }
    }

    lastPosition = nextPos;
    if (ev.isGUI())
        updateDisplayString();
    fixIfHostGetsOutside();
    emitMobilityStateChangedSignal();
    updateVisualRepresentation();
}

TraCIScenarioManager *TraCIMobility::getManager() const
{
    if (!manager)
        manager = check_and_cast<TraCIScenarioManager *>(const_cast<TraCIMobility *>(this)->getModuleByPath(par("traciScenarioManagerModule"))); //TODO: getModuleByPath is not const
    return manager;
}

void TraCIMobility::updateDisplayString()
{
    ASSERT(-M_PI <= angle);
    ASSERT(angle < M_PI);

    getParentModule()->getDisplayString().setTagArg("b", 2, "rect");
    getParentModule()->getDisplayString().setTagArg("b", 3, "red");
    getParentModule()->getDisplayString().setTagArg("b", 4, "red");
    getParentModule()->getDisplayString().setTagArg("b", 5, "0");

    if (angle < -M_PI + 0.5 * M_PI_4 * 1) {
        getParentModule()->getDisplayString().setTagArg("t", 0, "\u2190");
        getParentModule()->getDisplayString().setTagArg("b", 0, "4");
        getParentModule()->getDisplayString().setTagArg("b", 1, "2");
    }
    else if (angle < -M_PI + 0.5 * M_PI_4 * 3) {
        getParentModule()->getDisplayString().setTagArg("t", 0, "\u2199");
        getParentModule()->getDisplayString().setTagArg("b", 0, "3");
        getParentModule()->getDisplayString().setTagArg("b", 1, "3");
    }
    else if (angle < -M_PI + 0.5 * M_PI_4 * 5) {
        getParentModule()->getDisplayString().setTagArg("t", 0, "\u2193");
        getParentModule()->getDisplayString().setTagArg("b", 0, "2");
        getParentModule()->getDisplayString().setTagArg("b", 1, "4");
    }
    else if (angle < -M_PI + 0.5 * M_PI_4 * 7) {
        getParentModule()->getDisplayString().setTagArg("t", 0, "\u2198");
        getParentModule()->getDisplayString().setTagArg("b", 0, "3");
        getParentModule()->getDisplayString().setTagArg("b", 1, "3");
    }
    else if (angle < -M_PI + 0.5 * M_PI_4 * 9) {
        getParentModule()->getDisplayString().setTagArg("t", 0, "\u2192");
        getParentModule()->getDisplayString().setTagArg("b", 0, "4");
        getParentModule()->getDisplayString().setTagArg("b", 1, "2");
    }
    else if (angle < -M_PI + 0.5 * M_PI_4 * 11) {
        getParentModule()->getDisplayString().setTagArg("t", 0, "\u2197");
        getParentModule()->getDisplayString().setTagArg("b", 0, "3");
        getParentModule()->getDisplayString().setTagArg("b", 1, "3");
    }
    else if (angle < -M_PI + 0.5 * M_PI_4 * 13) {
        getParentModule()->getDisplayString().setTagArg("t", 0, "\u2191");
        getParentModule()->getDisplayString().setTagArg("b", 0, "2");
        getParentModule()->getDisplayString().setTagArg("b", 1, "4");
    }
    else if (angle < -M_PI + 0.5 * M_PI_4 * 15) {
        getParentModule()->getDisplayString().setTagArg("t", 0, "\u2196");
        getParentModule()->getDisplayString().setTagArg("b", 0, "3");
        getParentModule()->getDisplayString().setTagArg("b", 1, "3");
    }
    else {
        getParentModule()->getDisplayString().setTagArg("t", 0, "\u2190");
        getParentModule()->getDisplayString().setTagArg("b", 0, "4");
        getParentModule()->getDisplayString().setTagArg("b", 1, "2");
    }
}

void TraCIMobility::fixIfHostGetsOutside()
{
    raiseErrorIfOutside();
}

double TraCIMobility::calculateCO2emission(double v, double a) const
{
    // Calculate CO2 emission parameters according to:
    // Cappiello, A. and Chabini, I. and Nam, E.K. and Lue, A. and Abou Zeid, M., "A statistical model of vehicle emissions and fuel consumption," IEEE 5th International Conference on Intelligent Transportation Systems (IEEE ITSC), pp. 801-809, 2002

    double A = 1000 * 0.1326;    // W/m/s
    double B = 1000 * 2.7384e-03;    // W/(m/s)^2
    double C = 1000 * 1.0843e-03;    // W/(m/s)^3
    double M = 1325.0;    // kg

    // power in W
    double P_tract = A * v + B * v * v + C * v * v * v + M * a * v;    // for sloped roads: +M*g*sin_theta*v

    /*
       // "Category 7 vehicle" (e.g. a '92 Suzuki Swift)
       double alpha = 1.01;
       double beta = 0.0162;
       double delta = 1.90e-06;
       double zeta = 0.252;
       double alpha1 = 0.985;
     */

    // "Category 9 vehicle" (e.g. a '94 Dodge Spirit)
    double alpha = 1.11;
    double beta = 0.0134;
    double delta = 1.98e-06;
    double zeta = 0.241;
    double alpha1 = 0.973;

    if (P_tract <= 0)
        return alpha1;
    return alpha + beta * v * 3.6 + delta * v * v * v * (3.6 * 3.6 * 3.6) + zeta * a * v;
}

} // namespace inet

#endif // ifdef WITH_TRACI

