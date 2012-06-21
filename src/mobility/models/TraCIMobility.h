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

#ifndef MOBILITY_TRACI_TRACIMOBILITY_H
#define MOBILITY_TRACI_TRACIMOBILITY_H
#ifdef WITH_TRACI

#include <string>
#include <fstream>
#include <list>
#include <stdexcept>

#include <omnetpp.h>

#include "MobilityBase.h"
#include "ModuleAccess.h"
#include "world/traci/TraCIScenarioManager.h"

/**
 * @brief
 * Used in modules created by the TraCIScenarioManager.
 *
 * This module relies on the TraCIScenarioManager for state updates
 * and can not be used on its own.
 *
 * See the Veins website <a href="http://veins.car2x.org/"> for a tutorial, documentation, and publications </a>.
 *
 * @author Christoph Sommer, David Eckhoff, Luca Bedogni, Bastian Halmos
 *
 * @see TraCIScenarioManager
 * @see TraCIScenarioManagerLaunchd
 *
 * @ingroup mobility
 */
class INET_API TraCIMobility : public MobilityBase
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

		TraCIMobility() : MobilityBase(), isPreInitialized(false) {}
		virtual void initialize(int stage);
		virtual void initializePosition();
		virtual void finish();
		virtual Coord getCurrentPosition() {
			return getPosition();
		}
		virtual Coord getCurrentSpeed() {
			Coord v = Coord(cos(getAngleRad()), -sin(getAngleRad()));
			return v * getSpeed();
		}

		virtual void handleSelfMessage(cMessage *msg);
		virtual void preInitialize(std::string external_id, const Coord& position, std::string road_id = "", double speed = -1, double angle = -1);
		virtual void nextPosition(const Coord& position, std::string road_id = "", double speed = -1, double angle = -1, TraCIScenarioManager::VehicleSignal signals = TraCIScenarioManager::VEH_SIGNAL_UNDEF);
		virtual void move();
		virtual void updateDisplayString();
		virtual void setExternalId(std::string external_id) {
			this->external_id = external_id;
		}
		virtual std::string getExternalId() const {
			if (external_id == "") throw cRuntimeError("TraCIMobility::getExternalId called with no external_id set yet");
			return external_id;
		}
		virtual Coord getPosition() const {
			return lastPosition;
		}
		virtual std::string getRoadId() const {
			if (road_id == "") throw cRuntimeError("TraCIMobility::getRoadId called with no road_id set yet");
			return road_id;
		}
		virtual double getSpeed() const {
			if (speed == -1) throw cRuntimeError("TraCIMobility::getSpeed called with no speed set yet");
			return speed;
		}
		virtual TraCIScenarioManager::VehicleSignal getSignals() const {
			if (signals == -1) throw cRuntimeError("TraCIMobility::getSignals called with no signals set yet");
			return signals;
		}
		/**
		 * returns angle in rads, 0 being east, with -M_PI <= angle < M_PI.
		 */
		virtual double getAngleRad() const {
			if (angle == M_PI) throw cRuntimeError("TraCIMobility::getAngleRad called with no angle set yet");
			return angle;
		}
		virtual TraCIScenarioManager* getManager() const {
			if (!manager) manager = TraCIScenarioManagerAccess().get();
			return manager;
		}
		void commandSetSpeedMode(int32_t bitset) {
			getManager()->commandSetSpeedMode(getExternalId(), bitset);
		}
		void commandSetSpeed(double speed) {
			getManager()->commandSetSpeed(getExternalId(), speed);
		}
		void commandChangeRoute(std::string roadId, double travelTime) {
			getManager()->commandChangeRoute(getExternalId(), roadId, travelTime);
		}

		void commandNewRoute(std::string roadId) {
		    getManager()->commandNewRoute(getExternalId(), roadId);
		}
		void commandParkVehicle() {
			getManager()->commandSetVehicleParking(getExternalId());
		}

		double commandDistanceRequest(Coord position1, Coord position2, bool returnDrivingDistance) {
			return getManager()->commandDistanceRequest(position1, position2, returnDrivingDistance);
		}
		void commandStopNode(std::string roadId, double pos, uint8_t laneid, double radius, double waittime) {
			return getManager()->commandStopNode(getExternalId(), roadId, pos, laneid, radius, waittime);
		}
		std::list<std::string> commandGetPolygonIds() {
			return getManager()->commandGetPolygonIds();
		}
		std::string commandGetPolygonTypeId(std::string polyId) {
			return getManager()->commandGetPolygonTypeId(polyId);
		}
		std::list<Coord> commandGetPolygonShape(std::string polyId) {
			return getManager()->commandGetPolygonShape(polyId);
		}
		void commandSetPolygonShape(std::string polyId, std::list<Coord> points) {
			getManager()->commandSetPolygonShape(polyId, points);
		}
		bool commandAddVehicle(std::string vehicleId, std::string vehicleTypeId, std::string routeId, std::string laneId, double emitPosition, double emitSpeed) {
			return getManager()->commandAddVehicle(vehicleId, vehicleTypeId, routeId, laneId, emitPosition, emitSpeed);
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

		std::string external_id; /**< updated by setExternalId() */

		simtime_t lastUpdate; /**< updated by nextPosition() */
		Coord nextPos; /**< updated by nextPosition() */
		std::string road_id; /**< updated by nextPosition() */
		double speed; /**< updated by nextPosition() */
		double angle; /**< updated by nextPosition() */
		TraCIScenarioManager::VehicleSignal signals; /**<updated by nextPosition() */

		cMessage* startAccidentMsg;
		cMessage* stopAccidentMsg;
		mutable TraCIScenarioManager* manager;
		double last_speed;

		virtual void fixIfHostGetsOutside(); /**< called after each read to check for (and handle) invalid positions */

		/**
		 * Returns the amount of CO2 emissions in grams/second, calculated for an average Car
		 * @param v speed in m/s
		 * @param a acceleration in m/s^2
		 * @returns emission in g/s
		 */
		double calculateCO2emission(double v, double a) const;
};

class TraCIMobilityAccess : public ModuleAccess<TraCIMobility>
{
	public:
		TraCIMobilityAccess() : ModuleAccess<TraCIMobility>("mobility") {};
};


#endif
#endif

