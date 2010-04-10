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

#include <sstream>

#define WANT_WINSOCK2
#include <platdep/sockets.h>
#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32) || defined(__CYGWIN__) || defined(_WIN64)
#include <ws2tcpip.h>
#else
#include <netinet/tcp.h>
#include <netdb.h>
#include <arpa/inet.h>
#endif
#define MYSOCKET (*(SOCKET*)socketPtr)

#include "world/traci/TraCIScenarioManager.h"
#include "world/traci/TraCIConstants.h"
#include "mobility/traci/TraCIMobility.h"

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
	std::string roiRoads_s = par("roiRoads");
	std::string roiRects_s = par("roiRects");

	// parse roiRoads
	roiRoads.clear();
	std::istringstream roiRoads_i(roiRoads_s);
	std::string road;
	while (std::getline(roiRoads_i, road, ' ')) {
		roiRoads.push_back(road);
	}

	// parse roiRects
	roiRects.clear();
	std::istringstream roiRects_i(roiRects_s);
	std::string rect;
	while (std::getline(roiRects_i, rect, ' ')) {
		std::istringstream rect_i(rect);
		double x1; rect_i >> x1; if (!rect_i) error("parse error in roiRects");
		char c1; rect_i >> c1;
		double y1; rect_i >> y1; if (!rect_i) error("parse error in roiRects");
		char c2; rect_i >> c2;
		double x2; rect_i >> x2; if (!rect_i) error("parse error in roiRects");
		char c3; rect_i >> c3;
		double y2; rect_i >> y2; if (!rect_i) error("parse error in roiRects");
		roiRects.push_back(std::pair<Coord, Coord>(Coord(x1,y1), Coord(x2, y2)));
	}

	nextNodeVectorIndex = 0;
	hosts.clear();

	executeOneTimestepTrigger = new cMessage("step");
	scheduleAt(0, executeOneTimestepTrigger);

	cc = dynamic_cast<ChannelControl *>(simulation.getModuleByPath("channelcontrol"));
	if (cc == 0) error("Could not find a ChannelControl module named channelcontrol");

	if (debug) EV << "TraCIScenarioManager connecting to TraCI server" << endl;
	socketPtr = 0;
	connect();
	init_traci();

	if (debug) EV << "initialized TraCIScenarioManager" << endl;
}

std::string TraCIScenarioManager::receiveTraCIMessage() {
	if (!socketPtr) error("Connection to TraCI server lost");

	uint32_t msgLength;
	{
		char buf2[sizeof(uint32_t)];
		uint32_t bytesRead = 0;
		while (bytesRead < sizeof(uint32_t)) {
			int receivedBytes = ::recv(MYSOCKET, reinterpret_cast<char*>(&buf2) + bytesRead, sizeof(uint32_t) - bytesRead, 0);
			if (receivedBytes > 0) {
				bytesRead += receivedBytes;
			} else {
				if (errno == EINTR) continue;
				if (errno == EAGAIN) continue;
				error("Could not read %d bytes from TraCI server, got only %d: %s", sizeof(uint32_t), bytesRead, strerror(sock_errno()));
			}
		}
		TraCIBuffer(std::string(buf2, sizeof(uint32_t))) >> msgLength;
	}

	uint32_t bufLength = msgLength - sizeof(msgLength);
	char buf[bufLength];
	{
		if (debug) EV << "Reading TraCI message of " << bufLength << " bytes" << endl;
		uint32_t bytesRead = 0;
		while (bytesRead < bufLength) {
			int receivedBytes = ::recv(MYSOCKET, reinterpret_cast<char*>(&buf) + bytesRead, bufLength - bytesRead, 0);
			if (receivedBytes > 0) {
				bytesRead += receivedBytes;
			} else {
				if (errno == EINTR) continue;
				if (errno == EAGAIN) continue;
				error("Could not read %d bytes from TraCI server, got only %d: %s", bufLength, bytesRead, strerror(errno));
			}
		}
	}
	return std::string(buf, bufLength);
}

void TraCIScenarioManager::sendTraCIMessage(std::string buf) {
	if (!socketPtr) error("Connection to TraCI server lost");

	{
		uint32_t msgLength = sizeof(uint32_t) + buf.length();
		TraCIBuffer buf2 = TraCIBuffer(); buf2 << msgLength;
		size_t sentBytes = ::send(MYSOCKET, buf2.str().c_str(), sizeof(uint32_t), 0);
		if (sentBytes != sizeof(uint32_t)) error("Could not write %d bytes to TraCI server, sent only %d: %s", sizeof(uint32_t), sentBytes, strerror(errno));
	}

	{
		if (debug) EV << "Writing TraCI message of " << buf.length() << " bytes" << endl;
		size_t sentBytes = ::send(MYSOCKET, buf.c_str(), buf.length(), 0);
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

bool TraCIScenarioManager::queryTraCIOptional(uint8_t commandId, const TraCIBuffer& buf, std::string* errorMsg) {
	sendTraCIMessage(makeTraCICommand(commandId, buf));

	TraCIBuffer obuf(receiveTraCIMessage());
	uint8_t cmdLength; obuf >> cmdLength;
	uint8_t commandResp; obuf >> commandResp; if (commandResp != commandId) error("Expected response to command %d, but got one for command %d", commandId, commandResp);
	uint8_t result; obuf >> result;
	std::string description; obuf >> description;
	if (!obuf.eof()) error("expected only an OK/ERR response, but received additional bytes");
	return (result == RTYPE_OK);
}

void TraCIScenarioManager::connect() {
	if (initsocketlibonce() != 0) error("Could not init socketlib");

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

	socketPtr = new SOCKET();
	MYSOCKET = ::socket( AF_INET, SOCK_STREAM, 0 );
	if (MYSOCKET < 0) error("Could not create socket to connect to TraCI server");

	if (::connect(MYSOCKET, (sockaddr const*)&address, sizeof(address)) < 0) error("Could not connect to TraCI server");

	{
		int x = 1;
		::setsockopt(MYSOCKET, IPPROTO_TCP, TCP_NODELAY, (const char*)&x, sizeof(x));
	}
}

void TraCIScenarioManager::init_traci() {
	{
		// Send "Subscribe Lifecycles" Command
		uint8_t do_write = 0x00;
		uint8_t domain = DOM_ROADMAP;
		uint32_t objectId = 0;
		uint8_t variableId = DOMVAR_BOUNDINGBOX;
		uint8_t typeId = TYPE_BOUNDINGBOX;
		TraCIBuffer buf = queryTraCI(CMD_SCENARIO, TraCIBuffer() << do_write << domain << objectId << variableId << typeId);
		uint8_t cmdLength_resp; buf >> cmdLength_resp;
		uint8_t commandId_resp; buf >> commandId_resp; if (commandId_resp != CMD_SCENARIO) error("Expected response to CMD_SCENARIO, but got %d", commandId_resp);
		uint8_t do_write_resp; buf >> do_write_resp;
		uint8_t domain_resp; buf >> domain_resp;
		uint32_t objectId_resp; buf >> objectId_resp;
		uint8_t variableId_resp; buf >> variableId_resp;
		uint8_t typeId_resp; buf >> typeId_resp;
		float x1; buf >> x1;
		float y1; buf >> y1;
		float x2; buf >> x2;
		float y2; buf >> y2;
		netbounds1 = Coord(x1, y1);
		netbounds2 = Coord(x2, y2);
		if (debug) EV << "TraCI reports network boundaries (" << x1 << ", " << y1 << ")-(" << x2 << ", " << y2 << ")" << endl;
		if ((traci2omnet(netbounds2).x > cc->getPgs()->x) || (traci2omnet(netbounds2).y > cc->getPgs()->y)) EV << "WARNING: Playground size (" << cc->getPgs()->x << ", " << cc->getPgs()->y << ") might be too small for vehicle at network bounds (" << traci2omnet(netbounds2).x << ", " << traci2omnet(netbounds2).y << ")" << endl;
	}

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
	if (socketPtr) {
		closesocket(MYSOCKET);
		delete &MYSOCKET;
		socketPtr = 0;
	}
	while (hosts.begin() != hosts.end()) {
		deleteModule(hosts.begin()->first);
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

void TraCIScenarioManager::commandSetTrafficLightProgram(std::string trafficLightId, std::string program) {
	TraCIBuffer buf = queryTraCI(CMD_SET_TL_VARIABLE, TraCIBuffer() << static_cast<uint8_t>(TL_PROGRAM) << trafficLightId << static_cast<uint8_t>(TYPE_STRING) << program);
	if (!buf.eof()) error("expected only an OK response, but received additional bytes");
}

void TraCIScenarioManager::commandSetTrafficLightPhaseIndex(std::string trafficLightId, int32_t index) {
	TraCIBuffer buf = queryTraCI(CMD_SET_TL_VARIABLE, TraCIBuffer() << static_cast<uint8_t>(TL_PHASE_INDEX) << trafficLightId << static_cast<uint8_t>(TYPE_INTEGER) << index);
	if (!buf.eof()) error("expected only an OK response, but received additional bytes");
}

std::list<std::pair<float, float> > TraCIScenarioManager::commandGetPolygonShape(std::string polyId) {
	std::list<std::pair<float, float> > res;

	TraCIBuffer buf = queryTraCI(CMD_GET_POLYGON_VARIABLE, TraCIBuffer() << static_cast<uint8_t>(VAR_SHAPE) << polyId);

	// read additional RESPONSE_GET_POLYGON_VARIABLE sent back in response
	uint8_t cmdLength; buf >> cmdLength;
	if (cmdLength == 0) {
		uint32_t cmdLengthX; buf >> cmdLengthX;
	}
	uint8_t commandId; buf >> commandId; if (commandId != RESPONSE_GET_POLYGON_VARIABLE) error("Expected response type RESPONSE_GET_POLYGON_VARIABLE, but got %d", commandId);
	uint8_t varId; buf >> varId; if (varId != VAR_SHAPE) error("Expected response variable VAR_SHAPE, but got %d", varId);
	std::string polyId_r; buf >> polyId_r; if (polyId_r != polyId) error("Received response for wrong polyId: expected %s, but got %s", polyId.c_str(), polyId_r.c_str());
	uint8_t resType_r; buf >> resType_r; if (resType_r != TYPE_POLYGON) error("Received wrong response type: expected %d, but got %d", TYPE_POLYGON, resType_r);
	uint8_t count; buf >> count;
	for (uint8_t i = 0; i < count; i++) {
		float x; buf >> x;
		float y; buf >> y;
		Coord pos = traci2omnet(Coord(x, y));
		res.push_back(std::make_pair(pos.x, pos.y));
	}

	if (!buf.eof()) error("received additional bytes");

	return res;
}

void TraCIScenarioManager::commandSetPolygonShape(std::string polyId, std::list<std::pair<float, float> > points) {
	TraCIBuffer buf;
	uint8_t count = static_cast<uint8_t>(points.size());
	buf << static_cast<uint8_t>(VAR_SHAPE) << polyId << static_cast<uint8_t>(TYPE_POLYGON) << count;
	for (std::list<std::pair<float, float> >::const_iterator i = points.begin(); i != points.end(); ++i) {
		float x = i->first;
		float y = i->second;
		Coord pos = omnet2traci(Coord(x, y));
		buf << static_cast<float>(pos.x) << static_cast<float>(pos.y);
	}
	TraCIBuffer obuf = queryTraCI(CMD_SET_POLYGON_VARIABLE, buf);
	if (!obuf.eof()) error("received additional bytes");
}

// name: host;Car;i=vehicle.gif
void TraCIScenarioManager::addModule(int32_t nodeId, std::string type, std::string name, std::string displayString, const Coord& position, std::string road_id, double speed, double angle, double allowed_speed) {
	if (hosts.find(nodeId) != hosts.end()) error("tried adding duplicate module");

	int32_t nodeVectorIndex = nextNodeVectorIndex++;

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

	// pre-initialize TraCIMobility
	for (cModule::SubmoduleIterator iter(mod); !iter.end(); iter++) {
		cModule* submod = iter();
		TraCIMobility* mm = dynamic_cast<TraCIMobility*>(submod);
		if (!mm) continue;
		mm->preInitialize(nodeId, position, road_id, speed, angle, allowed_speed);
	}

	mod->callInitialize();
	hosts[nodeId] = mod;
}

cModule* TraCIScenarioManager::getManagedModule(int32_t nodeId) {
		if (hosts.find(nodeId) == hosts.end()) return 0;
		return hosts[nodeId];
}

void TraCIScenarioManager::deleteModule(int32_t nodeId) {
	cModule* mod = getManagedModule(nodeId);
	if (!mod) error("no vehicle with Id %d found", nodeId);

	if (!mod->getSubmodule("notificationBoard")) error("host has no submodule notificationBoard");
	cc->unregisterHost(mod);

	hosts.erase(nodeId);
	mod->callFinish();
	mod->deleteModule();
}

bool TraCIScenarioManager::isInRegionOfInterest(const Coord& position, std::string road_id, double speed, double angle, double allowed_speed) {
	if ((roiRoads.size() == 0) && (roiRects.size() == 0)) return true;
	if (roiRoads.size() > 0) {
		for (std::list<std::string>::const_iterator i = roiRoads.begin(); i != roiRoads.end(); ++i) {
			if (road_id == *i) return true;
		}
	}
	if (roiRects.size() > 0) {
		for (std::list<std::pair<Coord, Coord> >::const_iterator i = roiRects.begin(); i != roiRects.end(); ++i) {
			if ((position.x >= i->first.x) && (position.y >= i->first.y) && (position.x <= i->second.x) && (position.y <= i->second.y)) return true;
		}
	}
	return false;
}

void TraCIScenarioManager::processObjectCreation(uint8_t domain, int32_t nodeId) {
	if (domain != DOM_VEHICLE) error("Expected DOM_VEHICLE, but got %d", domain);

	// actual object creation is done in processUpdateObject
}

void TraCIScenarioManager::processObjectDestruction(uint8_t domain, int32_t nodeId) {
	if (domain != DOM_VEHICLE) error("Expected DOM_VEHICLE, but got %d", domain);

	// check if this object has been deleted already (e.g. because it was outside the ROI)
	cModule* mod = getManagedModule(nodeId);
	if (!mod) return;

	deleteModule(nodeId);

	if (debug) EV << "Removed vehicle #" << nodeId << endl;
}

void TraCIScenarioManager::processUpdateObject(uint8_t domain, int32_t nodeId, TraCIBuffer& buf) {
	if (domain != DOM_VEHICLE) error("Expected DOM_VEHICLE, but got %d", domain);

	double targetTime; buf >> targetTime; targetTime = targetTime;

	float px; buf >> px;
	float py; buf >> py;
	Coord p = traci2omnet(Coord(px, py));
	if ((p.x < 0) || (p.y < 0)) error("received bad node position (%.2f, %.2f), translated to (%.2f, %.2f)", px, py, p.x, p.y);

	std::string edge; buf >> edge;

	float speed; buf >> speed;

	float angle_traci; buf >> angle_traci;
	float angle = traci2omnetAngle(angle_traci);

	float allowed_speed; buf >> allowed_speed;

	cModule* mod = getManagedModule(nodeId);

	// is it in the ROI?
	bool inRoi = isInRegionOfInterest(Coord(px, py), edge, speed, angle, allowed_speed);
	if (!inRoi) {
		if (mod) {
			deleteModule(nodeId);
			if (debug) EV << "Vehicle #" << nodeId << " left region of interest" << endl;
		}
		return;
	}

	if (!mod) {
		// no such module - need to create
		addModule(nodeId, moduleType, moduleName, moduleDisplayString, p, edge, speed, angle, allowed_speed);
		if (debug) EV << "Added vehicle #" << nodeId << endl;
	} else {
		// module existed - update position
		for (cModule::SubmoduleIterator iter(mod); !iter.end(); iter++) {
			cModule* submod = iter();
			TraCIMobility* mm = dynamic_cast<TraCIMobility*>(submod);
			if (!mm) continue;
			if (debug) EV << "module " << nodeId << " moving to " << p.x << "," << p.y << endl;
			mm->nextPosition(p, edge, speed, angle, allowed_speed);
		}
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
	return Coord(coord.x - netbounds1.x + margin, (netbounds2.y - netbounds1.y) - (coord.y - netbounds1.y) + margin);
}

Coord TraCIScenarioManager::omnet2traci(Coord coord) const {
	return Coord(coord.x + netbounds1.x - margin, (netbounds2.y - netbounds1.y) - (coord.y - netbounds1.y) + margin);
}

double TraCIScenarioManager::traci2omnetAngle(double angle) const {

	// convert to rad	
	angle = angle * M_PI / 180.0;

	// rotate angle so 0 is east (in TraCI's angle interpretation 0 is south)
	angle = 1.5*M_PI - angle; 

	// normalize angle to -M_PI <= angle < M_PI
	while (angle < -M_PI) angle += 2*M_PI;
	while (angle >= M_PI) angle -= 2*M_PI;

	return angle;
}

double TraCIScenarioManager::omnet2traciAngle(double angle) const {

	// rotate angle so 0 is south (in OMNeT++'s angle interpretation 0 is east)
	angle = 1.5*M_PI - angle; 

	// convert to degrees
	angle = angle * 180 / M_PI;

	// normalize angle to 0 <= angle < 360
	while (angle < 0) angle += 360;
	while (angle >= 360) angle -= 360;

	return angle;
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

