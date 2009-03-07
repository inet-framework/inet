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
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>

#include <omnetpp.h>

#include "INETDefs.h"
#include "ChannelControl.h"
#include "ModuleAccess.h"

// hack for cygwin installations
#ifndef MSG_WAITALL
#define MSG_WAITALL 0x100
#endif

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
		virtual bool isTraCISimulationEnded();

		void commandSetMaximumSpeed(int32_t nodeId, float maxSpeed);
		void commandChangeRoute(int32_t nodeId, std::string roadId, double travelTime);

	protected:

		bool debug; /**< whether to emit debug messages */
		simtime_t updateInterval; /**< time interval to update the host's position */
		std::string moduleType; /**< module type to be used in the simulation for each managed vehicle */
		std::string moduleName; /**< module name to be used in the simulation for each managed vehicle */
		std::string moduleDisplayString; /**< module displayString to be used in the simulation for each managed vehicle */
		std::string host;
		int port;
		bool autoShutdown; /**< Shutdown module as soon as no more vehicles are in the simulation */

		int socket;
		long statsSimStart;
		int currStep;

		bool traCISimulationEnded;
		int packetNo; /**< current packet number (for debugging) */
		std::map<int32_t, cModule*> hosts; /**< vector of all hosts managed by us */
		std::map<int32_t, simtime_t> lastUpdate; /**< vector of all hosts' last update time */
		cMessage* executeOneTimestepTrigger; /**< self-message scheduled for when to next call executeOneTimestep */

		ChannelControl* cc;

		void executeOneTimestep(); /**< read and execute all commands for the next timestep */

		cModule* getManagedModule(int32_t nodeId); /**< returns a pointer to the managed module named moduleName, or 0 if no module can be found */

		void connect();
		void addModule(int32_t nodeId, std::string type, std::string name, std::string displayString);

		uint32_t commandSimStep(simtime_t targetTime, uint8_t positionType);

		bool socketDataAvailable() {
			int result;
			fd_set readset;
			struct timeval timeout;
			timeout.tv_sec = 1;
			timeout.tv_usec = 0;

			do {
				FD_ZERO(&readset);
				FD_SET(socket, &readset);
				result = select(socket + 1, &readset, NULL, NULL, &timeout);
			} while (result == -1 && errno == EINTR);

			if (result < 0) error("An error occured polling data from the TraCI server: %s", strerror(errno));

			return FD_ISSET(socket, &readset);
		}

		template<typename T> T readFromSocket() {
			if(traCISimulationEnded) error("Simulation has ended");
			if (socket < 0) error("Connection to TraCI server lost");

			T buf;
			int receivedBytes = ::recv(socket, reinterpret_cast<char*>(&buf), sizeof(buf), MSG_WAITALL);
			if (receivedBytes != sizeof(buf)) error("Could not read %d bytes from TraCI server, got only %d: %s", sizeof(buf), receivedBytes, strerror(errno));

			T buf_to_return;
        		short a = 0x0102;
        		unsigned char *p_a = reinterpret_cast<unsigned char*>(&a);
        		bool isBigEndian = (p_a[0] == 0x01);
			if (isBigEndian) {
				buf_to_return = buf;
			} else {
                		unsigned char *p_buf = reinterpret_cast<unsigned char*>(&buf);
		                unsigned char *p_buf_to_return = reinterpret_cast<unsigned char*>(&buf_to_return);

				for (size_t i=0; i<sizeof(buf); ++i)
				{
					p_buf_to_return[i] = p_buf[sizeof(buf)-1-i];
				}
			}

			return buf_to_return;
		}

		template<typename T> T readFromSocket(T& out) {
			out = readFromSocket<T>();
			return out;
		}

		template<typename T> void writeToSocket(T buf) {
			if(traCISimulationEnded) error("Simulation has ended");
			if (socket < 0) error("Connection to TraCI server lost");

			T buf_to_send;
        		short a = 0x0102;
        		unsigned char *p_a = reinterpret_cast<unsigned char*>(&a);
        		bool isBigEndian = (p_a[0] == 0x01);
			if (isBigEndian) {
				buf_to_send = buf;
			} else {
                		unsigned char *p_buf = reinterpret_cast<unsigned char*>(&buf);
		                unsigned char *p_buf_to_send = reinterpret_cast<unsigned char*>(&buf_to_send);

				for (size_t i=0; i<sizeof(buf); ++i)
				{
					p_buf_to_send[i] = p_buf[sizeof(buf)-1-i];
				}
			}

			int sentBytes = ::send(socket, reinterpret_cast<char*>(&buf_to_send), sizeof(buf), 0);
			if (sentBytes != sizeof(buf)) error("Could not write %d bytes to TraCI server, sent only %d: %s", sizeof(buf), sentBytes, strerror(errno));
		}
};

template<> std::string TraCIScenarioManager::readFromSocket();
template<> void TraCIScenarioManager::writeToSocket(std::string buf);

class TraCIScenarioManagerAccess : public ModuleAccess<TraCIScenarioManager>
{
	public:
		TraCIScenarioManagerAccess() : ModuleAccess<TraCIScenarioManager>("manager") {};
};

#endif
