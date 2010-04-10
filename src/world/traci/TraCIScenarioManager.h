//
// TraCIScenarioManager - connects OMNeT++ to a TraCI server, manages hosts
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

#ifndef WORLD_TRACI_TRACISCENARIOMANAGER_H
#define WORLD_TRACI_TRACISCENARIOMANAGER_H

#include <fstream>
#include <vector>
#include <map>
#include <list>
#include <stdexcept>

#include <omnetpp.h>

#include "INETDefs.h"
#include "ChannelControl.h"
#include "ModuleAccess.h"

/**
 * TraCIScenarioManager connects OMNeT++ to a TraCI server running road traffic simulations.
 * It sets up and controls simulation experiments, moving nodes with the help
 * of a TraCIMobility module.
 *
 * Last tested with SUMO r5488 (2008-04-30)
 * https://sumo.svn.sourceforge.net/svnroot/sumo/trunk/sumo
 *
 * @author Christoph Sommer
 */
class INET_API TraCIScenarioManager : public cSimpleModule
{
	public:

		~TraCIScenarioManager();
		virtual void initialize();
		virtual void finish();
		virtual void handleMessage(cMessage *msg);
		virtual void handleSelfMsg(cMessage *msg);

		void commandSetMaximumSpeed(int32_t nodeId, float maxSpeed);
		void commandChangeRoute(int32_t nodeId, std::string roadId, double travelTime);
		float commandDistanceRequest(Coord position1, Coord position2, bool returnDrivingDistance);
		void commandStopNode(int32_t nodeId, std::string roadId, float pos, uint8_t laneid, float radius, double waittime);
		void commandSetTrafficLightProgram(std::string trafficLightId, std::string program);
		void commandSetTrafficLightPhaseIndex(std::string trafficLightId, int32_t index);
		std::list<std::pair<float, float> > commandGetPolygonShape(std::string polyId);
		void commandSetPolygonShape(std::string polyId, std::list<std::pair<float, float> > points);

	protected:

		bool debug; /**< whether to emit debug messages */
		simtime_t updateInterval; /**< time interval to update the host's position */
		std::string moduleType; /**< module type to be used in the simulation for each managed vehicle */
		std::string moduleName; /**< module name to be used in the simulation for each managed vehicle */
		std::string moduleDisplayString; /**< module displayString to be used in the simulation for each managed vehicle */
		std::string host;
		int port;
		bool autoShutdown; /**< Shutdown module as soon as no more vehicles are in the simulation */
		int margin;
		std::list<std::string> roiRoads; /**< which roads (e.g. "hwy1 hwy2") are considered to consitute the region of interest, if not empty */
		std::list<std::pair<Coord, Coord> > roiRects; /**< which rectangles (e.g. "0,0-10,10 20,20-30,30) are considered to consitute the region of interest, if not empty */

		void* socketPtr;
		Coord netbounds1; /* network boundaries as reported by TraCI (x1, y1) */
		Coord netbounds2; /* network boundaries as reported by TraCI (x2, y2) */

		size_t nextNodeVectorIndex; /**< next OMNeT++ module vector index to use */
		std::map<int32_t, cModule*> hosts; /**< vector of all hosts managed by us */
		cMessage* executeOneTimestepTrigger; /**< self-message scheduled for when to next call executeOneTimestep */

		ChannelControl* cc;

		void executeOneTimestep(); /**< read and execute all commands for the next timestep */

		void connect();
		virtual void init_traci();

		void addModule(int32_t nodeId, std::string type, std::string name, std::string displayString, const Coord& position, std::string road_id = "", double speed = -1, double angle = -1, double allowed_speed = -1);
		cModule* getManagedModule(int32_t nodeId); /**< returns a pointer to the managed module named moduleName, or 0 if no module can be found */
		void deleteModule(int32_t nodeId);

		/**
		 * returns whether a given position lies within the simulation's region of interest.
		 * Modules are destroyed and re-created as managed vehicles leave and re-enter the ROI
		 */
		bool isInRegionOfInterest(const Coord& position, std::string road_id, double speed, double angle, double allowed_speed);

		/**
		 * Byte-buffer that stores values in TraCI byte-order
		 */
		class TraCIBuffer {
			public:
				TraCIBuffer() : buf() {
					buf_index = 0;
				}

				TraCIBuffer(std::string buf) : buf(buf) {
					buf_index = 0;
				}

				template<typename T> T read() {
					T buf_to_return;
					unsigned char *p_buf_to_return = reinterpret_cast<unsigned char*>(&buf_to_return);

					if (isBigEndian()) {
						for (size_t i=0; i<sizeof(buf_to_return); ++i) {
							if (eof()) throw std::runtime_error("Attempted to read past end of byte buffer");
							p_buf_to_return[i] = buf[buf_index++];
						}
					} else {
						for (size_t i=0; i<sizeof(buf_to_return); ++i) {
							if (eof()) throw std::runtime_error("Attempted to read past end of byte buffer");
							p_buf_to_return[sizeof(buf_to_return)-1-i] = buf[buf_index++];
						}
					}

					return buf_to_return;
				}

				template<typename T> void write(T inv) {
					unsigned char *p_buf_to_send = reinterpret_cast<unsigned char*>(&inv);

					if (isBigEndian()) {
						for (size_t i=0; i<sizeof(inv); ++i) {
							buf += p_buf_to_send[i];
						}
					} else {
						for (size_t i=0; i<sizeof(inv); ++i) {
							buf += p_buf_to_send[sizeof(inv)-1-i];
						}
					}
				}

				template<typename T> T read(T& out) {
					out = read<T>();
					return out;
				}

				template<typename T> TraCIBuffer& operator >>(T& out) {
					out = read<T>();
					return *this;
				}

				template<typename T> TraCIBuffer& operator <<(const T& inv) {
					write(inv);
					return *this;
				}

				bool eof() const {
					return buf_index == buf.length();
				}

				void set(std::string buf) {
					this->buf = buf;
					buf_index = 0;
				}

				void clear() {
					set("");
				}

				std::string str() const {
					return buf;
				}

			protected:
				bool isBigEndian() {
					short a = 0x0102;
					unsigned char *p_a = reinterpret_cast<unsigned char*>(&a);
					return (p_a[0] == 0x01);
				}

				std::string buf;
				size_t buf_index;
		};

		void processObjectCreation(uint8_t domain, int32_t nodeId);

		void processObjectDestruction(uint8_t domain, int32_t nodeId);

		void processUpdateObject(uint8_t domain, int32_t nodeId, TraCIBuffer& buf);

		/**
		 * sends a single command via TraCI, checks status response, returns additional responses
		 */
		TraCIBuffer queryTraCI(uint8_t commandId, const TraCIBuffer& buf = TraCIBuffer());

		/**
		 * sends a single command via TraCI, expects no reply, returns true if successful
		 */
		bool queryTraCIOptional(uint8_t commandId, const TraCIBuffer& buf, std::string* errorMsg = 0);

		/**
		 * returns byte-buffer containing a TraCI command with optional parameters
		 */
		std::string makeTraCICommand(uint8_t commandId, TraCIBuffer buf = TraCIBuffer());

		/**
		 * sends a message via TraCI (after adding the header)
		 */
		void sendTraCIMessage(std::string buf);

		/**
		 * receives a message via TraCI (and strips the header)
		 */
		std::string receiveTraCIMessage();

		/**
		 * convert TraCI coordinates to OMNeT++ coordinates
		 */
		Coord traci2omnet(Coord coord) const;

		/**
		 * convert OMNeT++ coordinates to TraCI coordinates
		 */
		Coord omnet2traci(Coord coord) const;

		/**
		 * convert TraCI angle to OMNeT++ angle (in rad)
		 */
		double traci2omnetAngle(double angle) const;

		/**
		 * convert OMNeT++ angle (in rad) to TraCI angle
		 */
		double omnet2traciAngle(double angle) const;

};

template<> void TraCIScenarioManager::TraCIBuffer::write(std::string inv);
template<> std::string TraCIScenarioManager::TraCIBuffer::read();

class TraCIScenarioManagerAccess : public ModuleAccess<TraCIScenarioManager>
{
	public:
		TraCIScenarioManagerAccess() : ModuleAccess<TraCIScenarioManager>("manager") {};
};

#endif
