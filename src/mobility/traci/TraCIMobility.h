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

#ifndef MOBILITY_TRACI_TRACIMOBILITY_H
#define MOBILITY_TRACI_TRACIMOBILITY_H

#include <string>
#include <fstream>
#include <list>
#include <stdexcept>

#include <omnetpp.h>

#include "BasicMobility.h"
#include "ModuleAccess.h"
#include "world/traci/TraCIScenarioManager.h"

/**
 * @brief
 * TraCIMobility is a mobility module for hosts controlled by TraCIScenarioManager.
 * It receives position and state updates from an external module and updates
 * the parent module accordingly.
 * See NED file for more info.
 *
 * @ingroup mobility
 * @author Christoph Sommer
 */
class INET_API TraCIMobility : public BasicMobility
{
	public:
		class Statistics {
			public:
				double firstRoadNumber; /**< for statistics: number of first road we encountered (if road id can be expressed as a number) */
				simtime_t startTime; /**< for statistics: start time */
				simtime_t totalTime; /**< for statistics: total time travelled */
				simtime_t stopTime; /**< for statistics: stop time */
				double minSpeed; /**< for statistics: minimum value of currentSpeed */
				double maxSpeed; /**< for statistics: maximum value of currentSpeed */
				double totalDistance; /**< for statistics: total distance travelled */
				double totalCO2Emission; /**< for statistics: total CO2 emission */

				void initialize();
				void watch(cSimpleModule& module);
				void recordScalars(cSimpleModule& module);
		};

		TraCIMobility() : BasicMobility(), isPreInitialized(false) {}
		virtual void initialize(int);
		virtual void finish();

		virtual void handleSelfMsg(cMessage *msg);
		virtual void preInitialize(int32_t external_id, const Coord& position, std::string road_id = "", double speed = -1, double angle = -1, double allowed_speed = -1);
		virtual void nextPosition(const Coord& position, std::string road_id = "", double speed = -1, double angle = -1, double allowed_speed = -1);
		virtual void changePosition();
		virtual void setExternalId(int32_t external_id) {
			this->external_id = external_id;
		}
		virtual int32_t getExternalId() {
			if (external_id == -1) throw std::runtime_error("TraCIMobility::getExternalId called with no external_id set yet");
			return external_id;
		}
		virtual Coord getPosition() {
			return pos;
		}
		virtual std::string getRoadId() {
			if (road_id == "") throw std::runtime_error("TraCIMobility::getRoadId called with no road_id set yet");
			return road_id;
		}
		virtual double getSpeed() {
			if (speed == -1) throw std::runtime_error("TraCIMobility::getSpeed called with no speed set yet");
			return speed;
		}
		/**
		 * returns angle in rads, 0 being east, with -M_PI <= angle < M_PI. 
		 */
		virtual double getAngleRad() {
			if (angle == M_PI) throw std::runtime_error("TraCIMobility::getAngleRad called with no angle set yet");
			return angle;
		}
		virtual double getAllowedSpeed() {
			if (allowed_speed == -1) throw std::runtime_error("TraCIMobility::getAllowedSpeed called with no allowed speed set yet");
			return allowed_speed;
		}
		virtual TraCIScenarioManager* getManager() {
			if (!manager) manager = TraCIScenarioManagerAccess().get();
			return manager;
		}
		void commandSetMaximumSpeed(float maxSpeed) {
			getManager()->commandSetMaximumSpeed(getExternalId(), maxSpeed);
		}
		void commandChangeRoute(std::string roadId, double travelTime) {
			getManager()->commandChangeRoute(getExternalId(), roadId, travelTime);
		}
		float commandDistanceRequest(Coord position1, Coord position2, bool returnDrivingDistance) {
			return getManager()->commandDistanceRequest(position1, position2, returnDrivingDistance);
		}
		void commandStopNode(std::string roadId, float pos, uint8_t laneid, float radius, double waittime) {
			return getManager()->commandStopNode(getExternalId(), roadId, pos, laneid, radius, waittime);
		}
		std::list<std::pair<float, float> > commandGetPolygonShape(std::string polyId) {
			return getManager()->commandGetPolygonShape(polyId);
		}
		void commandSetPolygonShape(std::string polyId, std::list<std::pair<float, float> > points) {
			getManager()->commandSetPolygonShape(polyId, points);
		}

	protected:
		bool debug; /**< whether to emit debug messages */
		int accidentCount; /**< number of accidents */

		cOutVector currentPosXVec; /**< vector plotting posx */
		cOutVector currentPosYVec; /**< vector plotting posy */
		cOutVector currentSpeedVec; /**< vector plotting speed */
		cOutVector currentAccelerationVec; /**< vector plotting acceleration */
		cOutVector currentCO2EmissionVec; /**< vector plotting current CO2 emission */

		Statistics statistics; /**< everything statistics-related */

		bool isPreInitialized; /**< true if preInitialize() has been called immediately before initialize() */

		int32_t external_id; /**< updated by setExternalId() */

		simtime_t lastUpdate; /**< updated by nextPosition() */
		Coord nextPos; /**< updated by nextPosition() */
		std::string road_id; /**< updated by nextPosition() */
		double speed; /**< updated by nextPosition() */
		double angle; /**< updated by nextPosition() */
		double allowed_speed; /**< updated by nextPosition() */

		cMessage* startAccidentMsg;
		cMessage* stopAccidentMsg;
		TraCIScenarioManager* manager;
		double last_speed;

		virtual void fixIfHostGetsOutside(); /**< called after each read to check for (and handle) invalid positions */

		/**
		 * Returns the amount of CO2 emissions in grams/second, calculated for an average Car
		 * @param v speed in m/s
		 * @param a acceleration in m/s^2
		 * @returns emission in g/s
		 */
		double calculateCO2emission(double v, double a);
};

class TraCIMobilityAccess : public ModuleAccess<TraCIMobility>
{
	public:
		TraCIMobilityAccess() : ModuleAccess<TraCIMobility>("mobility") {};
};


#endif

