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

#include "world/traci/TraCIScenarioManager.h"
#include "world/traci/TraCIConstants.h"
#include "mobility/traci/TraCIMobility.h"

#include <sstream>


Define_Module(TraCIScenarioManager);

TraCIScenarioManager::~TraCIScenarioManager()
{
	cancelAndDelete(executeOneTimestepTrigger);
}


void TraCIScenarioManager::initialize()
{
	debug = par("debug");
	updateInterval = par("updateInterval");
	moduleType = par("moduleType").stdstringValue();
	moduleName = par("moduleName").stdstringValue();
	moduleDisplayString = par("moduleDisplayString").stdstringValue();
	host = par("host").stdstringValue();
	port = par("port");
	autoShutdown = par("autoShutdown");

	packetNo = 0;

	traCISimulationEnded = false;
	executeOneTimestepTrigger = new cMessage("step");
	scheduleAt(0, executeOneTimestepTrigger);

	cc = dynamic_cast<ChannelControl *>(simulation.getModuleByPath("channelcontrol"));
	if (cc == 0) error("Could not find a ChannelControl module named channelcontrol");

	statsSimStart = time(NULL);
	currStep = 0;

	if (debug) EV << "TraCIScenarioManager connecting to TraCI server" << endl;
	socket = -1;
	connect();

	if (debug) EV << "initialized TraCIScenarioManager" << endl;
}

void TraCIScenarioManager::connect() {
	in_addr addr;
	struct hostent* host_ent;
	struct in_addr saddr;

	saddr.s_addr = inet_addr(host.c_str());
	if (saddr.s_addr != static_cast<unsigned int>(-1)) {
		addr = saddr;
	} else if ((host_ent = gethostbyname(host.c_str()))) {
		addr = *((struct in_addr*)host_ent->h_addr_list[0]);
	} else {
		error("Invalid TraCI server address: %s", host.c_str());
		return;
	}

	sockaddr_in address;
	memset( (char*)&address, 0, sizeof(address) );
	address.sin_family = AF_INET;
	address.sin_port = htons(port);
	address.sin_addr.s_addr = addr.s_addr;

	socket = ::socket( AF_INET, SOCK_STREAM, 0 );
	if (socket < 0) error("Could not create socket to connect to TraCI server");

	if (::connect(socket, (sockaddr const*)&address, sizeof(address)) < 0) error("Could not connect to TraCI server");

	{
		// Send "Subscribe Lifecycles" Command
		uint8_t command = CMD_SUBSCRIBELIFECYCLES;
		uint8_t domain = DOM_VEHICLE;
		uint8_t cmdLength = sizeof(cmdLength) + sizeof(command) + sizeof(domain);
		uint32_t msgLength = sizeof(msgLength) + cmdLength;
		writeToSocket(msgLength);
		writeToSocket(cmdLength);
		writeToSocket(command);
		writeToSocket(domain);

		{
			// Wait for Response
			uint32_t msgLength; readFromSocket(msgLength);
			uint8_t cmdLength; readFromSocket(cmdLength);
			uint8_t commandResp; readFromSocket(commandResp); if (commandResp != command) error("Expected response to %d, but got %d", command, commandResp);
			uint8_t result; readFromSocket(result); if (result != RTYPE_OK) error("Received non-OK response from TraCI server");
			std::string description; readFromSocket(description);

			uint32_t addlBytes = msgLength - sizeof(msgLength) - cmdLength;
			if (addlBytes != 0) error("expected only an OK response, but received %d additional bytes", addlBytes);
		}
	}

	{
		// Send "Subscribe Domain" Command for DOM_VEHICLE
		uint8_t command = CMD_SUBSCRIBEDOMAIN;
		uint8_t domain = DOM_VEHICLE;
		uint8_t variableCount = 5;
		uint8_t variableId1 = DOMVAR_SIMTIME;
		uint8_t dataType1 = TYPE_DOUBLE;
		uint8_t variableId2 = DOMVAR_POSITION;
		uint8_t dataType2 = POSITION_2D;
		uint8_t variableId3 = DOMVAR_POSITION;
		uint8_t dataType3 = POSITION_ROADMAP;
		uint8_t variableId4 = DOMVAR_SPEED;
		uint8_t dataType4 = TYPE_FLOAT;
		uint8_t variableId5 = DOMVAR_ANGLE;
		uint8_t dataType5 = TYPE_FLOAT;
		uint8_t cmdLength = sizeof(cmdLength) + sizeof(command) + sizeof(domain) + sizeof(variableCount) + sizeof(variableId1) + sizeof(dataType1) + sizeof(variableId2) + sizeof(dataType2) + sizeof(variableId3) + sizeof(dataType3) + sizeof(variableId4) + sizeof(dataType4) + sizeof(variableId5) + sizeof(dataType5);
		uint32_t msgLength = sizeof(msgLength) + cmdLength;
		writeToSocket(msgLength);
		writeToSocket(cmdLength);
		writeToSocket(command);
		writeToSocket(domain);
		writeToSocket(variableCount);
		writeToSocket(variableId1);
		writeToSocket(dataType1);
		writeToSocket(variableId2);
		writeToSocket(dataType2);
		writeToSocket(variableId3);
		writeToSocket(dataType3);
		writeToSocket(variableId4);
		writeToSocket(dataType4);
		writeToSocket(variableId5);
		writeToSocket(dataType5);

		{
			// Wait for Response
			uint32_t msgLength; readFromSocket(msgLength);
			uint8_t cmdLength; readFromSocket(cmdLength);
			uint8_t commandResp; readFromSocket(commandResp); if (commandResp != command) error("Expected response to %d, but got %d", command, commandResp);
			uint8_t result; readFromSocket(result); if (result != RTYPE_OK) error("Received non-OK response from TraCI server");
			std::string description; readFromSocket(description);

			uint32_t addlBytes = msgLength - sizeof(msgLength) - cmdLength;
			if (addlBytes != 0) error("expected only an OK response, but received %d additional bytes", addlBytes);
		}
	}


}

void TraCIScenarioManager::finish()
{
	if (executeOneTimestepTrigger->isScheduled()) {
		cancelEvent(executeOneTimestepTrigger);
		delete executeOneTimestepTrigger;
		executeOneTimestepTrigger = 0;
	}
	if (socket >= 0) {
		::close(socket);
		socket = -1;
	}
}

void TraCIScenarioManager::handleMessage(cMessage *msg)
{
	if (msg->isSelfMessage()) {
		handleSelfMsg(msg);
		return;
	}
	error("TraCIScenarioManager doesn't handle messages from other modules");
}

void TraCIScenarioManager::handleSelfMsg(cMessage *msg)
{
	if (msg == executeOneTimestepTrigger) {
		executeOneTimestep();
		return;
	}
	error("TraCIScenarioManager received unknown self-message");
}

cModule* TraCIScenarioManager::getManagedModule(int32_t nodeId) {
		if (hosts.find(nodeId) == hosts.end()) return 0;
		return hosts[nodeId];
}

bool TraCIScenarioManager::isTraCISimulationEnded() {
	return traCISimulationEnded;
}

void TraCIScenarioManager::commandSetMaximumSpeed(int32_t nodeId, float maxSpeed) {

	// Send Command
	uint8_t cmdLength = sizeof(uint8_t) + sizeof(uint8_t) + sizeof(nodeId) + sizeof(maxSpeed);
	uint32_t msgLength = sizeof(uint32_t) + cmdLength;
	writeToSocket(msgLength);
	writeToSocket(cmdLength);
	writeToSocket(static_cast<uint8_t>(CMD_SETMAXSPEED));
	writeToSocket(nodeId);
	writeToSocket(maxSpeed);

	// Wait for Response
	readFromSocket(msgLength);
	readFromSocket(cmdLength);
	uint8_t commandId; readFromSocket(commandId); if (commandId != CMD_SETMAXSPEED) error("Expected response to CMD_SETMAXSPEED, but got %d", commandId);
	uint8_t result; readFromSocket(result); if (result != RTYPE_OK) error("Received non-OK response from TraCI server");
	std::string description; readFromSocket(description);

	uint32_t addlBytes = msgLength - sizeof(msgLength) - cmdLength;
	if (addlBytes != 0) error("expected only an OK response, but received %d additional bytes", addlBytes);
}

void TraCIScenarioManager::commandChangeRoute(int32_t nodeId, std::string roadId, double travelTime) {

	// Send Command
	uint8_t cmdLength = sizeof(uint8_t) + sizeof(uint8_t) + sizeof(nodeId) + sizeof(uint32_t) + (sizeof(uint8_t)*roadId.length()) + sizeof(travelTime);
	uint32_t msgLength = sizeof(uint32_t) + cmdLength;
	writeToSocket(msgLength);
	writeToSocket(cmdLength);
	writeToSocket(static_cast<uint8_t>(CMD_CHANGEROUTE));
	writeToSocket(nodeId);
	writeToSocket(roadId);
	writeToSocket(travelTime);

	// Wait for Response
	readFromSocket(msgLength);
	readFromSocket(cmdLength);
	uint8_t commandId; readFromSocket(commandId); if (commandId != CMD_CHANGEROUTE) error("Expected response to CMD_CHANGEROUTE, but got %d", commandId);
	uint8_t result; readFromSocket(result); if (result != RTYPE_OK) error("Received non-OK response from TraCI server");
	std::string description; readFromSocket(description);

	uint32_t addlBytes = msgLength - sizeof(msgLength) - cmdLength;
	if (addlBytes != 0) error("expected only an OK response, but received %d additional bytes", addlBytes);

}

uint32_t TraCIScenarioManager::commandSimStep(simtime_t targetTime, uint8_t positionType) {

	// Send Command
	uint8_t cmdLength = sizeof(uint8_t) + sizeof(uint8_t) + sizeof(targetTime) + sizeof(positionType);
	uint32_t msgLength = sizeof(uint32_t) + cmdLength;
	writeToSocket(msgLength);
	writeToSocket(cmdLength);
	writeToSocket(static_cast<uint8_t>(CMD_SIMSTEP));
	writeToSocket(static_cast<double>(targetTime.dbl()));
	writeToSocket(positionType);

	// Wait for Response
	readFromSocket(msgLength);
	readFromSocket(cmdLength);
	uint8_t commandId; readFromSocket(commandId); if (commandId != CMD_SIMSTEP) error("Expected response to CMD_SIMSTEP, but got %d", commandId);
	uint8_t result; readFromSocket(result); if (result != RTYPE_OK) error("Received non-OK response from TraCI server");
	std::string description; readFromSocket(description);

	uint32_t addlBytes = msgLength - sizeof(msgLength) - cmdLength;
	return addlBytes;
}

// name: host;Car;i=vehicle.gif
void TraCIScenarioManager::addModule(int32_t nodeId, std::string type, std::string name, std::string displayString) {
	if (hosts.find(nodeId) != hosts.end()) error("tried adding duplicate module");

	uint32_t nodeVectorIndex = nodeId;

	cModule* parentmod = getParentModule();
	if (!parentmod) error("Parent Module not found");

	cModuleType* nodeType = cModuleType::get(type.c_str());
	if (!nodeType) error("Module Type \"%s\" not found", type.c_str());

	//TODO: this trashes the vectsize member of the cModule, although nobody seems to use it
	cModule* mod = nodeType->create(name.c_str(), parentmod, nodeVectorIndex, nodeVectorIndex);
	mod->finalizeParameters();
	mod->getDisplayString().parse(displayString.c_str());
	mod->buildInside();
	mod->scheduleStart(simTime()+updateInterval);
	mod->callInitialize();
	hosts[nodeId] = mod;
}

void TraCIScenarioManager::executeOneTimestep() {

	if (debug) EV << "Triggering TraCI server simulation advance to t=" << simTime() << endl;

	uint32_t bytesToRead = commandSimStep(simTime(), POSITION_NONE);
	uint32_t bytesTotal = bytesToRead;
	bool shutdown = false;

	while (bytesToRead > 0) {
		if (debug) EV << "Reading " << bytesToRead << "/" << bytesTotal << " bytes from TraCI server" << endl;
		uint8_t cmdLength; readFromSocket(cmdLength); bytesToRead -= cmdLength;
		uint8_t commandId; readFromSocket(commandId);

		if (commandId == CMD_OBJECTCREATION) {
			uint8_t domain; readFromSocket(domain);
			int32_t nodeId; readFromSocket(nodeId);
			if (domain != DOM_VEHICLE) error("Expected DOM_VEHICLE, but got %d", domain);

			cModule* mod = getManagedModule(nodeId);
			if (mod) error("Tried adding duplicate vehicle with Id %d", nodeId);

			addModule(nodeId, moduleType, moduleName, moduleDisplayString);
			mod = getManagedModule(nodeId);
			//if (debug) EV << "finding mm for: module creation of " << nodeId << endl;
			for (cModule::SubmoduleIterator iter(mod); !iter.end(); iter++) {
				cModule* submod = iter();
				TraCIMobility* mm = dynamic_cast<TraCIMobility*>(submod);
				if (!mm) continue;
				mm->setExternalId(nodeId);
			}
			if (debug) EV << "Added vehicle #" << nodeId << endl;
		} else if (commandId == CMD_OBJECTDESTRUCTION) {
			uint8_t domain; readFromSocket(domain);
			int32_t nodeId; readFromSocket(nodeId);
			if (domain != DOM_VEHICLE) error("Expected DOM_VEHICLE, but got %d", domain);

			cModule* mod = getManagedModule(nodeId);
			if (!mod) error("no vehicle with Id %d found", nodeId);

			if (!mod->getSubmodule("notificationBoard")) error("host has no submodule notificationBoard");
			cc->unregisterHost(mod);

			hosts.erase(nodeId);
			mod->callFinish();
			mod->deleteModule();
			if (debug) EV << "Removed vehicle #" << nodeId << endl;

			if (autoShutdown && (hosts.size() < 1)) {
				if (debug) EV << "Simulation End: All vehicles have left the simulation." << std::endl;
				shutdown = true;
			}

		} else if (commandId == CMD_UPDATEOBJECT) {

			uint8_t domainId; readFromSocket(domainId);
			if (domainId != DOM_VEHICLE) error("Expected DOM_VEHICLE, but got %d", domainId);

			int32_t nodeId; readFromSocket(nodeId);

			double targetTime; readFromSocket(targetTime); targetTime = targetTime;

			float px; readFromSocket(px);
			float py; readFromSocket(py);
			int pxi = static_cast<int>(px);
			int pyi = static_cast<int>(py);
			if ((pxi < 0) || (pyi < 0)) error("received bad node position");
			pyi = cc->getPgs()->y - pyi;

			std::string edge; readFromSocket(edge);

			float speed; readFromSocket(speed);

			float angle; readFromSocket(angle);


			cModule* mod = getManagedModule(nodeId);

			if (!mod) error("Vehicle #%d not found", nodeId);

			//if (debug) EV << "finding mm for: module " << nodeId << " moving to " << pxi << "," << pyi << endl;
			for (cModule::SubmoduleIterator iter(mod); !iter.end(); iter++) {
				cModule* submod = iter();
				TraCIMobility* mm = dynamic_cast<TraCIMobility*>(submod);
				if (!mm) continue;
				//if (debug) EV << "module " << nodeId << " moving to " << pxi << "," << pyi << endl;
				mm->nextPosition(pxi, pyi, speed, angle * M_PI / 180.0, edge);
			}
		} else {
			error("Expected CMD_OBJECTCREATION, CMD_UPDATEOBJECT, CMD_OBJECTDESTRUCTION, but got %d", commandId);
		}
	}

	if (!shutdown) scheduleAt(simTime()+updateInterval, executeOneTimestepTrigger);

}

template<> std::string TraCIScenarioManager::readFromSocket() {
	if(traCISimulationEnded) error("Simulation has ended");
	if (socket < 0) error("Connection to TraCI server lost");

	int32_t length = readFromSocket<uint32_t>();
	if (length == 0) return std::string();
	char buf[length+1];
	int receivedBytes = ::recv(socket, buf, length, MSG_WAITALL);
	if (receivedBytes != length) error("Could not read %d bytes from TraCI server, got only %d: %s", length, receivedBytes, strerror(errno));
	buf[length] = 0;
	return buf;
}

template<> void TraCIScenarioManager::writeToSocket(std::string buf) {
	if(traCISimulationEnded) error("Simulation has ended");
	if (socket < 0) error("Connection to TraCI server lost");

	writeToSocket<uint32_t>(buf.length());

	if (buf.length() == 0) return;

	unsigned int sentBytes = ::send(socket, reinterpret_cast<const char*>(buf.c_str()), buf.length(), 0);
	if (sentBytes != buf.length()) error("Could not write %d bytes to TraCI server, sent only %d: %s", buf.length(), sentBytes, strerror(errno));
}

