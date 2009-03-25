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
	margin = par("margin");

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
	init_traci();

	if (debug) EV << "initialized TraCIScenarioManager" << endl;
}

std::string TraCIScenarioManager::receiveTraCIMessage() {
	if(traCISimulationEnded) error("Simulation has ended");
	if (socket < 0) error("Connection to TraCI server lost");

	uint32_t msgLength;
	{
		char buf2[sizeof(uint32_t)];
		size_t receivedBytes = ::recv(socket, reinterpret_cast<char*>(&buf2), sizeof(uint32_t), MSG_WAITALL);
		if (receivedBytes != sizeof(uint32_t)) error("Could not read %d bytes from TraCI server, got only %d: %s", sizeof(uint32_t), receivedBytes, strerror(errno));
		TraCIBuffer(std::string(buf2, sizeof(uint32_t))) >> msgLength;
	}

	uint32_t bufLength = msgLength - sizeof(msgLength);
	char buf[bufLength];
	{
		if (debug) EV << "Reading TraCI message of " << bufLength << " bytes" << endl;
		size_t receivedBytes = ::recv(socket, reinterpret_cast<char*>(&buf), bufLength, MSG_WAITALL);
		if (receivedBytes != bufLength) error("Could not read %d bytes from TraCI server, got only %d: %s", bufLength, receivedBytes, strerror(errno));
	}
	return std::string(buf, bufLength);
}

void TraCIScenarioManager::sendTraCIMessage(std::string buf) {
	if(traCISimulationEnded) error("Simulation has ended");
	if (socket < 0) error("Connection to TraCI server lost");

	{
		uint32_t msgLength = sizeof(uint32_t) + buf.length();
		TraCIBuffer buf2 = TraCIBuffer(); buf2 << msgLength;
		size_t sentBytes = ::send(socket, buf2.str().c_str(), sizeof(uint32_t), 0);
		if (sentBytes != sizeof(uint32_t)) error("Could not write %d bytes to TraCI server, sent only %d: %s", sizeof(uint32_t), sentBytes, strerror(errno));
	}

	{
		if (debug) EV << "Writing TraCI message of " << buf.length() << " bytes" << endl;
		size_t sentBytes = ::send(socket, buf.c_str(), buf.length(), 0);
		if (sentBytes != buf.length()) error("Could not write %d bytes to TraCI server, sent only %d: %s", buf.length(), sentBytes, strerror(errno));
	}
}

std::string TraCIScenarioManager::makeTraCICommand(uint8_t commandId, TraCIBuffer buf) {
	if (sizeof(uint8_t) + sizeof(uint8_t) + buf.str().length() > 0xFF) {
		uint32_t len = sizeof(uint8_t) + sizeof(uint32_t) + sizeof(uint8_t) + buf.str().length();
		return (TraCIBuffer() << static_cast<uint8_t>(0) << len << commandId).str() + buf.str();
	}
	uint8_t len = sizeof(uint8_t) + sizeof(uint8_t) + buf.str().length();
	return (TraCIBuffer() << len << commandId).str() + buf.str();
}

TraCIScenarioManager::TraCIBuffer TraCIScenarioManager::queryTraCI(uint8_t commandId, const TraCIBuffer& buf) {
	sendTraCIMessage(makeTraCICommand(commandId, buf));

	TraCIBuffer obuf(receiveTraCIMessage());
	uint8_t cmdLength; obuf >> cmdLength;
	uint8_t commandResp; obuf >> commandResp; if (commandResp != commandId) error("Expected response to command %d, but got one for command %d", commandId, commandResp);
	uint8_t result; obuf >> result;
	std::string description; obuf >> description;
	if (result != RTYPE_OK) error("Received non-OK response from TraCI server to command %d: %s", commandId, description.c_str());
	return obuf;
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
}

void TraCIScenarioManager::init_traci() {
	{
		// Send "Subscribe Lifecycles" Command
		uint8_t domain = DOM_VEHICLE;
		TraCIBuffer buf = queryTraCI(CMD_SUBSCRIBELIFECYCLES, TraCIBuffer() << domain);
		if (!buf.eof()) error("expected only an OK response, but received additional bytes");
	}

	{
		// Send "Subscribe Domain" Command for DOM_VEHICLE
		uint8_t domain = DOM_VEHICLE;
		uint8_t variableCount = 6;
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
		uint8_t variableId6 = DOMVAR_ALLOWED_SPEED;
		uint8_t dataType6 = TYPE_FLOAT;
		TraCIBuffer buf = queryTraCI(CMD_SUBSCRIBEDOMAIN, TraCIBuffer() << domain << variableCount << variableId1 << dataType1 << variableId2 << dataType2 << variableId3 << dataType3 << variableId4 << dataType4 << variableId5 << dataType5 << variableId6 << dataType6);
		if (!buf.eof()) error("expected only an OK response, but received additional bytes");
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

bool TraCIScenarioManager::isTraCISimulationEnded() const {
	return traCISimulationEnded;
}

void TraCIScenarioManager::commandSetMaximumSpeed(int32_t nodeId, float maxSpeed) {
	TraCIBuffer buf = queryTraCI(CMD_SETMAXSPEED, TraCIBuffer() << nodeId << maxSpeed);
	if (!buf.eof()) error("expected only an OK response, but received additional bytes");
}

void TraCIScenarioManager::commandChangeRoute(int32_t nodeId, std::string roadId, double travelTime) {
	TraCIBuffer buf = queryTraCI(CMD_CHANGEROUTE, TraCIBuffer() << nodeId << roadId << travelTime);
	if (!buf.eof()) error("expected only an OK response, but received additional bytes");
}

float TraCIScenarioManager::commandDistanceRequest(Coord position1, Coord position2, bool returnDrivingDistance)
{
	position1 = omnet2traci(position1);
	position2 = omnet2traci(position2);
	TraCIBuffer buf = queryTraCI(CMD_DISTANCEREQUEST, TraCIBuffer() << static_cast<uint8_t>(POSITION_2D) << float(position1.x) << float(position1.y) << static_cast<uint8_t>(POSITION_2D) << float(position2.x) << float(position2.y) << static_cast<uint8_t>(returnDrivingDistance ? REQUEST_DRIVINGDIST : REQUEST_AIRDIST));

	uint8_t cmdLength; buf >> cmdLength;
	uint8_t commandId; buf >> commandId;
	if (commandId != CMD_DISTANCEREQUEST)
	{
		error("Expected response to CMD_DISTANCEREQUEST, but got %d", commandId);
	}

	uint8_t flag; buf >> flag;
	if (flag != static_cast<uint8_t>(returnDrivingDistance ? REQUEST_DRIVINGDIST : REQUEST_AIRDIST))
	{
		error("Received wrong distance type: %x", flag);
	}

	float distance; buf >> distance;

	if (!buf.eof()) error("expected only a distance type and a distance, but received additional bytes");

	return distance;
}

void TraCIScenarioManager::commandStopNode(int32_t nodeId, std::string roadId, float pos, uint8_t laneid, float radius, double waittime) {
	TraCIBuffer buf = queryTraCI(CMD_STOP, TraCIBuffer() << nodeId << static_cast<uint8_t>(POSITION_ROADMAP) << roadId << pos << laneid << radius << waittime);

	// read additional CMD_STOP sent back in response
	uint8_t cmdLength; buf >> cmdLength;
	uint8_t commandId; buf >> commandId; if (commandId != CMD_STOP) error("Expected response to CMD_STOP, but got %d", commandId);
	int32_t nodeId_r; buf >> nodeId_r; if (nodeId_r != nodeId) error("Received response to CMD_STOP for wrong nodeId: expected %d, but got %d", nodeId, nodeId_r);
	uint8_t posType_r; buf >> posType_r; if (posType_r != POSITION_ROADMAP) error("Received response to CMD_STOP containing POSITION_ROADMAP: expected %d, but got %d", POSITION_ROADMAP, posType_r);
	std::string roadId_r; buf >> roadId_r; if (roadId_r != roadId) error("Received response to CMD_STOP for wrong roadId: expected %s, but got %s", roadId.c_str(), roadId_r.c_str());
	float pos_r; buf >> pos_r;
	uint8_t laneid_r; buf >> laneid_r;
	float radius_r; buf >> radius_r;
	double waittime_r; buf >> waittime_r;

	if (!buf.eof()) error("expected only a response to CMD_STOP, but received additional bytes");
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

void TraCIScenarioManager::processObjectCreation(uint8_t domain, int32_t nodeId) {
	if (domain != DOM_VEHICLE) error("Expected DOM_VEHICLE, but got %d", domain);

	cModule* mod = getManagedModule(nodeId);
	if (mod) error("Tried adding duplicate vehicle with Id %d", nodeId);

	addModule(nodeId, moduleType, moduleName, moduleDisplayString);
	mod = getManagedModule(nodeId);
	for (cModule::SubmoduleIterator iter(mod); !iter.end(); iter++) {
		cModule* submod = iter();
		TraCIMobility* mm = dynamic_cast<TraCIMobility*>(submod);
		if (!mm) continue;
		mm->setExternalId(nodeId);
	}
	if (debug) EV << "Added vehicle #" << nodeId << endl;
}

void TraCIScenarioManager::processObjectDestruction(uint8_t domain, int32_t nodeId) {
	if (domain != DOM_VEHICLE) error("Expected DOM_VEHICLE, but got %d", domain);

	cModule* mod = getManagedModule(nodeId);
	if (!mod) error("no vehicle with Id %d found", nodeId);

	if (!mod->getSubmodule("notificationBoard")) error("host has no submodule notificationBoard");
	cc->unregisterHost(mod);

	hosts.erase(nodeId);
	mod->callFinish();
	mod->deleteModule();
	if (debug) EV << "Removed vehicle #" << nodeId << endl;
}

void TraCIScenarioManager::processUpdateObject(uint8_t domain, int32_t nodeId, TraCIBuffer& buf) {
	if (domain != DOM_VEHICLE) error("Expected DOM_VEHICLE, but got %d", domain);

	double targetTime; buf >> targetTime; targetTime = targetTime;

	float px; buf >> px;
	float py; buf >> py;
	Coord p = traci2omnet(Coord(px, py)); px = p.x; py = p.y;
	int pxi = static_cast<int>(px);
	int pyi = static_cast<int>(py);
	if ((pxi < 0) || (pyi < 0)) error("received bad node position");

	std::string edge; buf >> edge;

	float speed; buf >> speed;

	float angle; buf >> angle;

	float allowed_speed; buf >> allowed_speed;

	cModule* mod = getManagedModule(nodeId);

	if (!mod) error("Vehicle #%d not found", nodeId);

	for (cModule::SubmoduleIterator iter(mod); !iter.end(); iter++) {
		cModule* submod = iter();
		TraCIMobility* mm = dynamic_cast<TraCIMobility*>(submod);
		if (!mm) continue;
		if (debug) EV << "module " << nodeId << " moving to " << pxi << "," << pyi << endl;
		mm->nextPosition(pxi, pyi, edge, speed, angle * M_PI / 180.0, allowed_speed);
	}
}

void TraCIScenarioManager::executeOneTimestep() {

	if (debug) EV << "Triggering TraCI server simulation advance to t=" << simTime() << endl;

	double targetTime = simTime().dbl();
	uint8_t positionType = POSITION_NONE;
	TraCIBuffer buf = queryTraCI(CMD_SIMSTEP, TraCIBuffer() << static_cast<double>(targetTime) << positionType);

	bool shutdown = false;
	while (!buf.eof()) {
		uint8_t cmdLength; buf >> cmdLength;
		uint8_t commandId; buf >> commandId;

		if (commandId == CMD_OBJECTCREATION) {
			uint8_t domain; buf >> domain;
			int32_t nodeId; buf >> nodeId;
			processObjectCreation(domain, nodeId);
		} else if (commandId == CMD_OBJECTDESTRUCTION) {
			uint8_t domain; buf >> domain;
			int32_t nodeId; buf >> nodeId;
			processObjectDestruction(domain, nodeId);
			if (autoShutdown && (hosts.size() < 1)) {
				if (debug) EV << "Simulation End: All vehicles have left the simulation." << std::endl;
				shutdown = true;
			}
		} else if (commandId == CMD_UPDATEOBJECT) {
			uint8_t domain; buf >> domain;
			int32_t nodeId; buf >> nodeId;
			processUpdateObject(domain, nodeId, buf);
		} else {
			error("Expected CMD_OBJECTCREATION, CMD_UPDATEOBJECT, CMD_OBJECTDESTRUCTION, but got %d", commandId);
		}
	}

	if (!shutdown) scheduleAt(simTime()+updateInterval, executeOneTimestepTrigger);

}

Coord TraCIScenarioManager::traci2omnet(Coord coord) const {
	return Coord(coord.x + margin, cc->getPgs()->y - (coord.y + margin));
}

Coord TraCIScenarioManager::omnet2traci(Coord coord) const {
	return Coord(coord.x - margin, cc->getPgs()->y - (coord.y + margin));
}

template<> void TraCIScenarioManager::TraCIBuffer::write(std::string inv) {
	uint32_t length = inv.length();
	write<uint32_t>(length);
	for (size_t i=0; i<length; ++i) write<char>(inv[i]);
}

template<> std::string TraCIScenarioManager::TraCIBuffer::read() {
	uint32_t length = read<uint32_t>();
	if (length == 0) return std::string();
	char obuf[length+1];

	for (size_t i=0; i<length; ++i) read<char>(obuf[i]);
	obuf[length] = 0;

	return std::string(obuf, length);
}

