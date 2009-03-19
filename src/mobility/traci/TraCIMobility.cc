//
// TraCIMobility - Mobility module to be controlled by TraCIScenarioManager
// Copyright (C) 2006 Christoph Sommer <christoph.sommer@informatik.uni-erlangen.de>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#include <limits>

#include "mobility/traci/TraCIMobility.h"

Define_Module(TraCIMobility);

namespace {
	const double MY_INFINITY = (std::numeric_limits<double>::has_infinity ? std::numeric_limits<double>::infinity() : std::numeric_limits<double>::max());
}

void TraCIMobility::initialize(int stage)
{
	BasicMobility::initialize(stage);

	if (stage == 1)
	{
		debug = par("debug");
		accidentCount = par("accidentCount");

		currentPosXVec.setName("posx");
		currentPosYVec.setName("posy");
		currentSpeedVec.setName("speed");
		currentAccelerationVec.setName("acceleration");
		currentCO2EmissionVec.setName("co2emission");

		startTime = simTime();
		totalTime = 0; WATCH(totalTime);
		stopTime = 0;
		minSpeed = MY_INFINITY; WATCH(minSpeed);
		maxSpeed = -MY_INFINITY; WATCH(maxSpeed);
		totalDistance = 0; WATCH(totalDistance);
		totalCO2Emission = 0;

		external_id = -1;

		nextPos = Coord(-1,-1);
		road_id = -1; WATCH(road_id);
		speed = -1; WATCH(speed);
		angle = -1; WATCH(angle);
		allowed_speed = -1; WATCH(allowed_speed);

		startAccidentMsg = 0;
		stopAccidentMsg = 0;
		manager = 0;
		last_speed = -1;


		pos.x = -1; WATCH(pos.x);
		pos.y = -1; WATCH(pos.y);

		if (accidentCount > 0) {
			simtime_t accidentStart = par("accidentStart");
			startAccidentMsg = new cMessage("scheduledAccident");
			stopAccidentMsg = new cMessage("scheduledAccidentResolved");
			scheduleAt(simTime() + accidentStart, startAccidentMsg);
		}
	}

}

void TraCIMobility::finish()
{
	stopTime = simTime();

	recordScalar("startTime", startTime);
	recordScalar("totalTime", totalTime);
	recordScalar("stopTime", stopTime);
	if (minSpeed != MY_INFINITY) recordScalar("minSpeed", minSpeed);
	if (maxSpeed != -MY_INFINITY) recordScalar("maxSpeed", maxSpeed);
	recordScalar("totalDistance", totalDistance);
	recordScalar("totalCO2Emission", totalCO2Emission);

	cancelAndDelete(startAccidentMsg);
	cancelAndDelete(stopAccidentMsg);
}

void TraCIMobility::handleSelfMsg(cMessage *msg)
{
	if (msg == startAccidentMsg) {
		commandSetMaximumSpeed(0);
		simtime_t accidentDuration = par("accidentDuration");
		scheduleAt(simTime() + accidentDuration, stopAccidentMsg);
		accidentCount--;
	}
	else if (msg == stopAccidentMsg) {
		commandSetMaximumSpeed(-1);
		if (accidentCount > 0) {
			simtime_t accidentInterval = par("accidentInterval");
			scheduleAt(simTime() + accidentInterval, startAccidentMsg);
		}
	}
}

void TraCIMobility::nextPosition(int x, int y, std::string road_id, double speed, double angle, double allowed_speed)
{
	if (debug) EV << "nextPosition " << x << " " << y << " " << road_id << " " << speed << " " << angle << " " << allowed_speed << std::endl;
	nextPos = Coord(x,y);
	this->road_id = road_id;
	this->speed = speed;
	this->angle = angle;
	this->allowed_speed = allowed_speed;
	changePosition();
}

void TraCIMobility::changePosition()
{
	simtime_t updateInterval = simTime() - this->lastUpdate;
	this->lastUpdate = simTime();

	// keep speed statistics
	if ((pos.x != -1) && (pos.y != -1)) {
		double distance = sqrt(((pos.x - nextPos.x) * (pos.x - nextPos.x)) + ((pos.y - nextPos.y) * (pos.y - nextPos.y)));
		totalDistance += distance;
		totalTime += updateInterval;
		if (speed != -1) {
			minSpeed = std::min(minSpeed, speed);
			maxSpeed = std::max(maxSpeed, speed);
			currentPosXVec.record(pos.x);
			currentPosYVec.record(pos.y);
			currentSpeedVec.record(speed);
			if (last_speed != -1) {
				double acceleration = (speed - last_speed) / updateInterval;
				double co2emission = calculateCO2emission(speed, acceleration);
				currentAccelerationVec.record(acceleration);
				currentCO2EmissionVec.record(co2emission);
				totalCO2Emission+=co2emission * updateInterval.dbl();
			}
			last_speed = speed;
		} else {
			last_speed = -1;
			speed = -1;
		}
	}

	pos.x = nextPos.x;
	pos.y = nextPos.y;
	fixIfHostGetsOutside();
	updatePosition();
}

void TraCIMobility::fixIfHostGetsOutside()
{
	raiseErrorIfOutside();
}

double TraCIMobility::calculateCO2emission(double v, double a) {
	// Calculate CO2 emission parameters according to:
	// Cappiello, A. and Chabini, I. and Nam, E.K. and Lue, A. and Abou Zeid, M., "A statistical model of vehicle emissions and fuel consumption," IEEE 5th International Conference on Intelligent Transportation Systems (IEEE ITSC), pp. 801-809, 2002

	double A = 1000 * 0.1326; // W/m/s
        double B = 1000 * 2.7384e-03; // W/(m/s)^2
	double C = 1000 * 1.0843e-03; // W/(m/s)^3
	double M = 1325.0; // kg

	// power in W
	double P_tract = A*v + B*v*v + C*v*v*v + M*a*v; // for sloped roads: +M*g*sin_theta*v

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

	if (P_tract <= 0) return alpha1;
	return alpha + beta*v*3.6 + delta*v*v*v*(3.6*3.6*3.6) + zeta*a*v;
}

