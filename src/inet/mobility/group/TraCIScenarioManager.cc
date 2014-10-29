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

#include <fstream>
#include <vector>
#include <algorithm>
#include <stdexcept>

#define WANT_WINSOCK2
#include <platdep/sockets.h>
#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32) || defined(__CYGWIN__) || defined(_WIN64)
#include <ws2tcpip.h>
#else // if defined(_WIN32) || defined(__WIN32__) || defined(WIN32) || defined(__CYGWIN__) || defined(_WIN64)
#include <netinet/tcp.h>
#include <netdb.h>
#include <arpa/inet.h>
#endif // if defined(_WIN32) || defined(__WIN32__) || defined(WIN32) || defined(__CYGWIN__) || defined(_WIN64)
#define MYSOCKET    (*(SOCKET *)socketPtr)

#include "inet/mobility/group/TraCIScenarioManager.h"
#include "inet/mobility/group/TraCIConstants.h"
#include "inet/mobility/group/TraCIMobility.h"

namespace inet {

Define_Module(TraCIScenarioManager);

TraCIScenarioManager::~TraCIScenarioManager()
{
    cancelAndDelete(connectAndStartTrigger);
    cancelAndDelete(executeOneTimestepTrigger);
}

void TraCIScenarioManager::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        connectAt = par("connectAt");
        firstStepAt = par("firstStepAt");
        updateInterval = par("updateInterval");
        if (firstStepAt == -1)
            firstStepAt = connectAt + updateInterval;
        if (firstStepAt <= connectAt)
            throw cRuntimeError("bad parameters: firstStepAt should be larger than connectAt");
        moduleType = par("moduleType").stdstringValue();
        moduleName = par("moduleName").stdstringValue();
        moduleDisplayString = par("moduleDisplayString").stdstringValue();
        penetrationRate = par("penetrationRate").doubleValue();
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
            double x1;
            rect_i >> x1;
            ASSERT(rect_i);
            char c1;
            rect_i >> c1;
            ASSERT(rect_i);
            double y1;
            rect_i >> y1;
            ASSERT(rect_i);
            char c2;
            rect_i >> c2;
            ASSERT(rect_i);
            double x2;
            rect_i >> x2;
            ASSERT(rect_i);
            char c3;
            rect_i >> c3;
            ASSERT(rect_i);
            double y2;
            rect_i >> y2;
            ASSERT(rect_i);
            roiRects.push_back(std::pair<TraCICoord, TraCICoord>(TraCICoord(x1, y1), TraCICoord(x2, y2)));
        }

        nextNodeVectorIndex = 0;
        hosts.clear();
        subscribedVehicles.clear();
        activeVehicleCount = 0;
        autoShutdownTriggered = false;

        socketPtr = 0;

        connectAndStartTrigger = new cMessage("connect");
        scheduleAt(connectAt, connectAndStartTrigger);
        executeOneTimestepTrigger = new cMessage("step");
        scheduleAt(firstStepAt, executeOneTimestepTrigger);

        EV_DEBUG << "initialized TraCIScenarioManager" << endl;
    }
}

std::string TraCIScenarioManager::receiveTraCIMessage()
{
    if (!socketPtr)
        throw cRuntimeError("Connection to TraCI server lost");

    uint32_t msgLength;
    {
        char buf2[sizeof(uint32_t)];
        uint32_t bytesRead = 0;
        while (bytesRead < sizeof(uint32_t)) {
            int receivedBytes = ::recv(MYSOCKET, reinterpret_cast<char *>(&buf2) + bytesRead, sizeof(uint32_t) - bytesRead, 0);
            if (receivedBytes > 0) {
                bytesRead += receivedBytes;
            }
            else if (receivedBytes == 0) {
                throw cRuntimeError("Connection to TraCI server closed unexpectedly. Check your server's log");
            }
            else {
                if (sock_errno() == EINTR)
                    continue;
                if (sock_errno() == EAGAIN)
                    continue;
                throw cRuntimeError("Connection to TraCI server lost. Check your server's log. Error message: %d: %s", sock_errno(), strerror(sock_errno()));
            }
        }
        TraCIBuffer(std::string(buf2, sizeof(uint32_t))) >> msgLength;
    }

    uint32_t bufLength = msgLength - sizeof(msgLength);
    char buf[bufLength];
    {
        EV_DEBUG << "Reading TraCI message of " << bufLength << " bytes" << endl;
        uint32_t bytesRead = 0;
        while (bytesRead < bufLength) {
            int receivedBytes = ::recv(MYSOCKET, reinterpret_cast<char *>(&buf) + bytesRead, bufLength - bytesRead, 0);
            if (receivedBytes > 0) {
                bytesRead += receivedBytes;
            }
            else if (receivedBytes == 0) {
                throw cRuntimeError("Connection to TraCI server closed unexpectedly. Check your server's log");
            }
            else {
                if (sock_errno() == EINTR)
                    continue;
                if (sock_errno() == EAGAIN)
                    continue;
                throw cRuntimeError("Connection to TraCI server lost. Check your server's log. Error message: %d: %s", sock_errno(), strerror(sock_errno()));
            }
        }
    }
    return std::string(buf, bufLength);
}

void TraCIScenarioManager::sendTraCIMessage(std::string buf)
{
    if (!socketPtr)
        throw cRuntimeError("Connection to TraCI server lost");

    {
        uint32_t msgLength = sizeof(uint32_t) + buf.length();
        TraCIBuffer buf2 = TraCIBuffer();
        buf2 << msgLength;
        uint32_t bytesWritten = 0;
        while (bytesWritten < sizeof(uint32_t)) {
            size_t sentBytes = ::send(MYSOCKET, buf2.str().c_str() + bytesWritten, sizeof(uint32_t) - bytesWritten, 0);
            if (sentBytes > 0) {
                bytesWritten += sentBytes;
            }
            else {
                if (sock_errno() == EINTR)
                    continue;
                if (sock_errno() == EAGAIN)
                    continue;
                throw cRuntimeError("Connection to TraCI server lost. Check your server's log. Error message: %d: %s", sock_errno(), strerror(sock_errno()));
            }
        }
    }

    {
        EV_DEBUG << "Writing TraCI message of " << buf.length() << " bytes" << endl;
        uint32_t bytesWritten = 0;
        while (bytesWritten < buf.length()) {
            size_t sentBytes = ::send(MYSOCKET, buf.c_str() + bytesWritten, buf.length() - bytesWritten, 0);
            if (sentBytes > 0) {
                bytesWritten += sentBytes;
            }
            else {
                if (sock_errno() == EINTR)
                    continue;
                if (sock_errno() == EAGAIN)
                    continue;
                throw cRuntimeError("Connection to TraCI server lost. Check your server's log. Error message: %d: %s", sock_errno(), strerror(sock_errno()));
            }
        }
    }
}

std::string TraCIScenarioManager::makeTraCICommand(uint8_t commandId, TraCIBuffer buf)
{
    if (sizeof(uint8_t) + sizeof(uint8_t) + buf.str().length() > 0xFF) {
        uint32_t len = sizeof(uint8_t) + sizeof(uint32_t) + sizeof(uint8_t) + buf.str().length();
        return (TraCIBuffer() << static_cast<uint8_t>(0) << len << commandId).str() + buf.str();
    }
    uint8_t len = sizeof(uint8_t) + sizeof(uint8_t) + buf.str().length();
    return (TraCIBuffer() << len << commandId).str() + buf.str();
}

TraCIScenarioManager::TraCIBuffer TraCIScenarioManager::queryTraCI(uint8_t commandId, const TraCIBuffer& buf)
{
    sendTraCIMessage(makeTraCICommand(commandId, buf));

    TraCIBuffer obuf(receiveTraCIMessage());
    uint8_t cmdLength;
    obuf >> cmdLength;
    uint8_t commandResp;
    obuf >> commandResp;
    ASSERT(commandResp == commandId);
    uint8_t result;
    obuf >> result;
    std::string description;
    obuf >> description;
    if (result == RTYPE_NOTIMPLEMENTED)
        throw cRuntimeError("TraCI server reported command 0x%2x not implemented (\"%s\"). Might need newer version.", commandId, description.c_str());
    if (result == RTYPE_ERR)
        throw cRuntimeError("TraCI server reported error executing command 0x%2x (\"%s\").", commandId, description.c_str());
    ASSERT(result == RTYPE_OK);
    return obuf;
}

TraCIScenarioManager::TraCIBuffer TraCIScenarioManager::queryTraCIOptional(uint8_t commandId, const TraCIBuffer& buf, bool& success, std::string *errorMsg)
{
    sendTraCIMessage(makeTraCICommand(commandId, buf));

    TraCIBuffer obuf(receiveTraCIMessage());
    uint8_t cmdLength;
    obuf >> cmdLength;
    uint8_t commandResp;
    obuf >> commandResp;
    ASSERT(commandResp == commandId);
    uint8_t result;
    obuf >> result;
    std::string description;
    obuf >> description;
    success = (result == RTYPE_OK);
    if (errorMsg)
        *errorMsg = description;
    return obuf;
}

void TraCIScenarioManager::connect()
{
    EV_DEBUG << "TraCIScenarioManager connecting to TraCI server" << endl;

    if (initsocketlibonce() != 0)
        throw cRuntimeError("Could not init socketlib");

    in_addr addr;
    struct hostent *host_ent;
    struct in_addr saddr;

    saddr.s_addr = inet_addr(host.c_str());
    if (saddr.s_addr != static_cast<unsigned int>(-1)) {
        addr = saddr;
    }
    else if ((host_ent = gethostbyname(host.c_str()))) {
        addr = *((struct in_addr *)host_ent->h_addr_list[0]);
    }
    else {
        throw cRuntimeError("Invalid TraCI server address: %s", host.c_str());
        return;
    }

    sockaddr_in address;
    memset((char *)&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = addr.s_addr;

    socketPtr = new SOCKET();
    MYSOCKET = ::socket(AF_INET, SOCK_STREAM, 0);
    if (MYSOCKET < 0)
        throw cRuntimeError("Could not create socket to connect to TraCI server");

    if (::connect(MYSOCKET, (sockaddr const *)&address, sizeof(address)) < 0) {
        throw cRuntimeError("Could not connect to TraCI server. Make sure it is running and not behind a firewall. Error message: %d: %s", sock_errno(), strerror(sock_errno()));
    }

    {
        int x = 1;
        ::setsockopt(MYSOCKET, IPPROTO_TCP, TCP_NODELAY, (const char *)&x, sizeof(x));
    }
}

void TraCIScenarioManager::init_traci()
{
    {
        std::pair<uint32_t, std::string> version = TraCIScenarioManager::commandGetVersion();
        uint32_t apiVersion = version.first;
        std::string serverVersion = version.second;

        if ((apiVersion == 2) || (apiVersion == 3)) {
            EV_DEBUG << "TraCI server \"" << serverVersion << "\" reports API version " << apiVersion << endl;
        }
        else {
            throw cRuntimeError("TraCI server \"%s\" reports API version %d. This server is unsupported.", serverVersion.c_str(), apiVersion);
        }
    }

    {
        // query road network boundaries
        TraCIBuffer buf = queryTraCI(CMD_GET_SIM_VARIABLE, TraCIBuffer() << static_cast<uint8_t>(VAR_NET_BOUNDING_BOX) << std::string("sim0"));
        uint8_t cmdLength_resp;
        buf >> cmdLength_resp;
        uint8_t commandId_resp;
        buf >> commandId_resp;
        ASSERT(commandId_resp == RESPONSE_GET_SIM_VARIABLE);
        uint8_t variableId_resp;
        buf >> variableId_resp;
        ASSERT(variableId_resp == VAR_NET_BOUNDING_BOX);
        std::string simId;
        buf >> simId;
        uint8_t typeId_resp;
        buf >> typeId_resp;
        ASSERT(typeId_resp == TYPE_BOUNDINGBOX);
        double x1;
        buf >> x1;
        double y1;
        buf >> y1;
        double x2;
        buf >> x2;
        double y2;
        buf >> y2;
        ASSERT(buf.eof());

        netbounds1 = TraCICoord(x1, y1);
        netbounds2 = TraCICoord(x2, y2);
        EV_DEBUG << "TraCI reports network boundaries (" << x1 << ", " << y1 << ")-(" << x2 << ", " << y2 << ")" << endl;
    }

    {
        // subscribe to list of departed and arrived vehicles, as well as simulation time
        uint32_t beginTime = 0;
        uint32_t endTime = 0x7FFFFFFF;
        std::string objectId = "";
        uint8_t variableNumber = 3;
        uint8_t variable1 = VAR_DEPARTED_VEHICLES_IDS;
        uint8_t variable2 = VAR_ARRIVED_VEHICLES_IDS;
        uint8_t variable3 = VAR_TIME_STEP;
        TraCIBuffer buf = queryTraCI(CMD_SUBSCRIBE_SIM_VARIABLE, TraCIBuffer() << beginTime << endTime << objectId << variableNumber << variable1 << variable2 << variable3);
        processSubcriptionResult(buf);
        ASSERT(buf.eof());
    }

    {
        // subscribe to list of vehicle ids
        uint32_t beginTime = 0;
        uint32_t endTime = 0x7FFFFFFF;
        std::string objectId = "";
        uint8_t variableNumber = 1;
        uint8_t variable1 = ID_LIST;
        TraCIBuffer buf = queryTraCI(CMD_SUBSCRIBE_VEHICLE_VARIABLE, TraCIBuffer() << beginTime << endTime << objectId << variableNumber << variable1);
        processSubcriptionResult(buf);
        ASSERT(buf.eof());
    }
}

void TraCIScenarioManager::finish()
{
    cancelAndDelete(executeOneTimestepTrigger);
    executeOneTimestepTrigger = NULL;
    cancelAndDelete(connectAndStartTrigger);
    connectAndStartTrigger = NULL;
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
    throw cRuntimeError("TraCIScenarioManager doesn't handle messages from other modules");
}

void TraCIScenarioManager::handleSelfMsg(cMessage *msg)
{
    if (msg == connectAndStartTrigger) {
        connect();
        init_traci();
        return;
    }
    if (msg == executeOneTimestepTrigger) {
        executeOneTimestep();
        return;
    }
    throw cRuntimeError("TraCIScenarioManager received unknown self-message");
}

std::pair<uint32_t, std::string> TraCIScenarioManager::commandGetVersion()
{
    bool success = false;
    TraCIBuffer buf = queryTraCIOptional(CMD_GETVERSION, TraCIBuffer(), success);

    if (!success) {
        ASSERT(buf.eof());
        return std::pair<uint32_t, std::string>(0, "(unknown)");
    }

    uint8_t cmdLength;
    buf >> cmdLength;
    uint8_t commandResp;
    buf >> commandResp;
    ASSERT(commandResp == CMD_GETVERSION);
    uint32_t apiVersion;
    buf >> apiVersion;
    std::string serverVersion;
    buf >> serverVersion;
    ASSERT(buf.eof());

    return std::pair<uint32_t, std::string>(apiVersion, serverVersion);
}

void TraCIScenarioManager::commandSetSpeedMode(std::string nodeId, int32_t bitset)
{
    uint8_t variableId = VAR_SPEEDSETMODE;
    uint8_t variableType = TYPE_INTEGER;
    TraCIBuffer buf = queryTraCI(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << bitset);
    ASSERT(buf.eof());
}

void TraCIScenarioManager::commandSetSpeed(std::string nodeId, double speed)
{
    uint8_t variableId = VAR_SPEED;
    uint8_t variableType = TYPE_DOUBLE;
    TraCIBuffer buf = queryTraCI(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << speed);
    ASSERT(buf.eof());
}

void TraCIScenarioManager::commandNewRoute(std::string nodeId, std::string roadId)
{
    uint8_t variableId = LANE_EDGE_ID;
    uint8_t variableType = TYPE_STRING;
    TraCIBuffer buf = queryTraCI(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << roadId);
    ASSERT(buf.eof());
}

void TraCIScenarioManager::commandSetVehicleParking(std::string nodeId)
{
    uint8_t variableId = REMOVE;
    uint8_t variableType = TYPE_BYTE;
    uint8_t value = NOTIFICATION_PARKING;
    TraCIBuffer buf = queryTraCI(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << value);
    ASSERT(buf.eof());
}

std::string TraCIScenarioManager::commandGetEdgeId(std::string nodeId)
{
    return genericGetString(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_ROAD_ID, RESPONSE_GET_VEHICLE_VARIABLE);
}

std::string TraCIScenarioManager::commandGetCurrentEdgeOnRoute(std::string nodeId)
{
    return genericGetString(CMD_GET_VEHICLE_VARIABLE, nodeId, LANE_EDGE_ID, RESPONSE_GET_VEHICLE_VARIABLE);
}

std::string TraCIScenarioManager::commandGetLaneId(std::string nodeId)
{
    return genericGetString(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_LANE_ID, RESPONSE_GET_VEHICLE_VARIABLE);
}

double TraCIScenarioManager::commandGetLanePosition(std::string nodeId)
{
    return genericGetDouble(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_LANEPOSITION, RESPONSE_GET_VEHICLE_VARIABLE);
}

std::list<std::string> TraCIScenarioManager::commandGetPlannedEdgeIds(std::string nodeId)
{
    return genericGetStringList(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_EDGES, RESPONSE_GET_VEHICLE_VARIABLE);
}

std::string TraCIScenarioManager::commandGetRouteId(std::string nodeId)
{
    return genericGetString(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_ROUTE_ID, RESPONSE_GET_VEHICLE_VARIABLE);
}

std::list<std::string> TraCIScenarioManager::commandGetRouteEdgeIds(std::string routeId)
{
    return genericGetStringList(CMD_GET_ROUTE_VARIABLE, routeId, VAR_EDGES, RESPONSE_GET_ROUTE_VARIABLE);
}

void TraCIScenarioManager::commandChangeRoute(std::string nodeId, std::string roadId, double travelTime)
{
    if (travelTime >= 0) {
        uint8_t variableId = VAR_EDGE_TRAVELTIME;
        uint8_t variableType = TYPE_COMPOUND;
        int32_t count = 2;
        uint8_t edgeIdT = TYPE_STRING;
        std::string edgeId = roadId;
        uint8_t newTimeT = TYPE_DOUBLE;
        double newTime = travelTime;
        TraCIBuffer buf = queryTraCI(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << count << edgeIdT << edgeId << newTimeT << newTime);
        ASSERT(buf.eof());
    }
    else {
        uint8_t variableId = VAR_EDGE_TRAVELTIME;
        uint8_t variableType = TYPE_COMPOUND;
        int32_t count = 1;
        uint8_t edgeIdT = TYPE_STRING;
        std::string edgeId = roadId;
        TraCIBuffer buf = queryTraCI(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << count << edgeIdT << edgeId);
        ASSERT(buf.eof());
    }
    {
        uint8_t variableId = CMD_REROUTE_TRAVELTIME;
        uint8_t variableType = TYPE_COMPOUND;
        int32_t count = 0;
        TraCIBuffer buf = queryTraCI(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << count);
        ASSERT(buf.eof());
    }
}

double TraCIScenarioManager::commandDistanceRequest(Coord position1, Coord position2, bool returnDrivingDistance)
{
    TraCICoord p1 = omnet2traci(position1);
    TraCICoord p2 = omnet2traci(position2);
    TraCIBuffer buf = queryTraCI(CMD_DISTANCEREQUEST, TraCIBuffer() << static_cast<uint8_t>(POSITION_2D) << double(p1.x) << double(p1.y) << static_cast<uint8_t>(POSITION_2D) << double(p2.x) << double(p2.y) << static_cast<uint8_t>(returnDrivingDistance ? REQUEST_DRIVINGDIST : REQUEST_AIRDIST));

    uint8_t cmdLength;
    buf >> cmdLength;
    uint8_t commandId;
    buf >> commandId;
    ASSERT(commandId == CMD_DISTANCEREQUEST);

    uint8_t flag;
    buf >> flag;
    ASSERT(flag == static_cast<uint8_t>(returnDrivingDistance ? REQUEST_DRIVINGDIST : REQUEST_AIRDIST));

    double distance;
    buf >> distance;

    ASSERT(buf.eof());

    return distance;
}

void TraCIScenarioManager::commandStopNode(std::string nodeId, std::string roadId, double pos, uint8_t laneid, double radius, double waittime)
{
    uint8_t variableId = CMD_STOP;
    uint8_t variableType = TYPE_COMPOUND;
    int32_t count = 4;
    uint8_t edgeIdT = TYPE_STRING;
    std::string edgeId = roadId;
    uint8_t stopPosT = TYPE_DOUBLE;
    double stopPos = pos;
    uint8_t stopLaneT = TYPE_BYTE;
    uint8_t stopLane = laneid;
    uint8_t durationT = TYPE_INTEGER;
    uint32_t duration = waittime * 1000;

    TraCIBuffer buf = queryTraCI(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << count << edgeIdT << edgeId << stopPosT << stopPos << stopLaneT << stopLane << durationT << duration);
    ASSERT(buf.eof());
}

void TraCIScenarioManager::commandSetTrafficLightProgram(std::string trafficLightId, std::string program)
{
    TraCIBuffer buf = queryTraCI(CMD_SET_TL_VARIABLE, TraCIBuffer() << static_cast<uint8_t>(TL_PROGRAM) << trafficLightId << static_cast<uint8_t>(TYPE_STRING) << program);
    ASSERT(buf.eof());
}

void TraCIScenarioManager::commandSetTrafficLightPhaseIndex(std::string trafficLightId, int32_t index)
{
    TraCIBuffer buf = queryTraCI(CMD_SET_TL_VARIABLE, TraCIBuffer() << static_cast<uint8_t>(TL_PHASE_INDEX) << trafficLightId << static_cast<uint8_t>(TYPE_INTEGER) << index);
    ASSERT(buf.eof());
}

std::list<std::string> TraCIScenarioManager::commandGetPolygonIds()
{
    return genericGetStringList(CMD_GET_POLYGON_VARIABLE, "", ID_LIST, RESPONSE_GET_POLYGON_VARIABLE);
}

std::string TraCIScenarioManager::commandGetPolygonTypeId(std::string polyId)
{
    return genericGetString(CMD_GET_POLYGON_VARIABLE, polyId, VAR_TYPE, RESPONSE_GET_POLYGON_VARIABLE);
}

std::list<Coord> TraCIScenarioManager::commandGetPolygonShape(std::string polyId)
{
    return genericGetCoordList(CMD_GET_POLYGON_VARIABLE, polyId, VAR_SHAPE, RESPONSE_GET_POLYGON_VARIABLE);
}

void TraCIScenarioManager::commandSetPolygonShape(std::string polyId, std::list<Coord> points)
{
    TraCIBuffer buf;
    uint8_t count = static_cast<uint8_t>(points.size());
    buf << static_cast<uint8_t>(VAR_SHAPE) << polyId << static_cast<uint8_t>(TYPE_POLYGON) << count;
    for (std::list<Coord>::const_iterator i = points.begin(); i != points.end(); ++i) {
        TraCICoord pos = omnet2traci(*i);
        buf << static_cast<double>(pos.x) << static_cast<double>(pos.y);
    }
    TraCIBuffer obuf = queryTraCI(CMD_SET_POLYGON_VARIABLE, buf);
    ASSERT(obuf.eof());
}

void TraCIScenarioManager::commandAddPolygon(std::string polyId, std::string polyType, const TraCIScenarioManager::Color& color, bool filled, int32_t layer, std::list<Coord> points)
{
    TraCIBuffer p;

    p << static_cast<uint8_t>(ADD) << polyId;
    p << static_cast<uint8_t>(TYPE_COMPOUND) << static_cast<int32_t>(5);
    p << static_cast<uint8_t>(TYPE_STRING) << polyType;
    p << static_cast<uint8_t>(TYPE_COLOR) << color.red << color.green << color.blue << color.alpha;
    p << static_cast<uint8_t>(TYPE_UBYTE) << static_cast<uint8_t>(filled);
    p << static_cast<uint8_t>(TYPE_INTEGER) << layer;
    p << static_cast<uint8_t>(TYPE_POLYGON) << static_cast<uint8_t>(points.size());
    for (std::list<Coord>::const_iterator i = points.begin(); i != points.end(); ++i) {
        TraCICoord pos = omnet2traci(*i);
        p << static_cast<double>(pos.x) << static_cast<double>(pos.y);
    }

    TraCIBuffer buf = queryTraCI(CMD_SET_POLYGON_VARIABLE, p);
    ASSERT(buf.eof());
}

std::list<std::string> TraCIScenarioManager::commandGetLaneIds()
{
    return genericGetStringList(CMD_GET_LANE_VARIABLE, "", ID_LIST, RESPONSE_GET_LANE_VARIABLE);
}

std::list<Coord> TraCIScenarioManager::commandGetLaneShape(std::string laneId)
{
    return genericGetCoordList(CMD_GET_LANE_VARIABLE, laneId, VAR_SHAPE, RESPONSE_GET_LANE_VARIABLE);
}

std::string TraCIScenarioManager::commandGetLaneEdgeId(std::string laneId)
{
    return genericGetString(CMD_GET_LANE_VARIABLE, laneId, LANE_EDGE_ID, RESPONSE_GET_LANE_VARIABLE);
}

double TraCIScenarioManager::commandGetLaneLength(std::string laneId)
{
    return genericGetDouble(CMD_GET_LANE_VARIABLE, laneId, VAR_LENGTH, RESPONSE_GET_LANE_VARIABLE);
}

double TraCIScenarioManager::commandGetLaneMaxSpeed(std::string laneId)
{
    return genericGetDouble(CMD_GET_LANE_VARIABLE, laneId, VAR_MAXSPEED, RESPONSE_GET_LANE_VARIABLE);
}

double TraCIScenarioManager::commandGetLaneMeanSpeed(std::string laneId)
{
    return genericGetDouble(CMD_GET_LANE_VARIABLE, laneId, LAST_STEP_MEAN_SPEED, RESPONSE_GET_LANE_VARIABLE);
}

std::list<std::string> TraCIScenarioManager::commandGetJunctionIds()
{
    return genericGetStringList(CMD_GET_JUNCTION_VARIABLE, "", ID_LIST, RESPONSE_GET_JUNCTION_VARIABLE);
}

Coord TraCIScenarioManager::commandGetJunctionPosition(std::string junctionId)
{
    return genericGetCoord(CMD_GET_JUNCTION_VARIABLE, junctionId, VAR_POSITION, RESPONSE_GET_JUNCTION_VARIABLE);
}

bool TraCIScenarioManager::commandAddVehicle(std::string vehicleId, std::string vehicleTypeId, std::string routeId, std::string laneId, float emitPosition, float emitSpeed)
{
    bool success = false;
    TraCIBuffer buf = queryTraCIOptional(CMD_ADDVEHICLE, TraCIBuffer() << vehicleId << vehicleTypeId << routeId << laneId << emitPosition << emitSpeed, success);
    ASSERT(buf.eof());
    if (success)
        activeVehicleCount++;
    return success;
}

// name: host;Car;i=vehicle.gif
void TraCIScenarioManager::addModule(std::string nodeId, std::string type, std::string name, std::string displayString, const Coord& position, std::string road_id, double speed, double angle)
{
    if (hosts.find(nodeId) != hosts.end())
        throw cRuntimeError("tried adding duplicate module");

    double option1 = hosts.size() / (hosts.size() + unEquippedHosts.size() + 1.0);
    double option2 = (hosts.size() + 1) / (hosts.size() + unEquippedHosts.size() + 1.0);

    if (fabs(option1 - penetrationRate) < fabs(option2 - penetrationRate)) {
        unEquippedHosts.insert(nodeId);
        return;
    }

    int32_t nodeVectorIndex = nextNodeVectorIndex++;

    cModule *parentmod = getParentModule();
    if (!parentmod)
        throw cRuntimeError("Parent Module not found");

    cModuleType *nodeType = cModuleType::get(type.c_str());
    if (!nodeType)
        throw cRuntimeError("Module Type \"%s\" not found", type.c_str());

    //TODO: this trashes the vectsize member of the cModule, although nobody seems to use it
    cModule *mod = nodeType->create(name.c_str(), parentmod, nodeVectorIndex, nodeVectorIndex);
    mod->finalizeParameters();
    mod->getDisplayString().parse(displayString.c_str());
    mod->buildInside();
    mod->scheduleStart(simTime() + updateInterval);

    // pre-initialize TraCIMobility
    for (cModule::SubmoduleIterator iter(mod); !iter.end(); iter++) {
        cModule *submod = iter();
        TraCIMobility *mm = dynamic_cast<TraCIMobility *>(submod);
        if (!mm)
            continue;
        mm->preInitialize(nodeId, position, road_id, speed, angle);
    }

    mod->callInitialize();
    hosts[nodeId] = mod;
}

cModule *TraCIScenarioManager::getManagedModule(std::string nodeId)
{
    if (hosts.find(nodeId) == hosts.end())
        return 0;
    return hosts[nodeId];
}

bool TraCIScenarioManager::isModuleUnequipped(std::string nodeId)
{
    if (unEquippedHosts.find(nodeId) == unEquippedHosts.end())
        return false;
    return true;
}

void TraCIScenarioManager::deleteModule(std::string nodeId)
{
    cModule *mod = getManagedModule(nodeId);
    if (!mod)
        throw cRuntimeError("no vehicle with Id \"%s\" found", nodeId.c_str());

    hosts.erase(nodeId);
    mod->callFinish();
    mod->deleteModule();
}

bool TraCIScenarioManager::isInRegionOfInterest(const TraCICoord& position, std::string road_id, double speed, double angle)
{
    if ((roiRoads.size() == 0) && (roiRects.size() == 0))
        return true;
    if (roiRoads.size() > 0) {
        for (std::list<std::string>::const_iterator i = roiRoads.begin(); i != roiRoads.end(); ++i) {
            if (road_id == *i)
                return true;
        }
    }
    if (roiRects.size() > 0) {
        for (std::list<std::pair<TraCICoord, TraCICoord> >::const_iterator i = roiRects.begin(); i != roiRects.end(); ++i) {
            if ((position.x >= i->first.x) && (position.y >= i->first.y) && (position.x <= i->second.x) && (position.y <= i->second.y))
                return true;
        }
    }
    return false;
}

uint32_t TraCIScenarioManager::getCurrentTimeMs()
{
    return static_cast<uint32_t>(round(simTime().dbl() * 1000));
}

void TraCIScenarioManager::executeOneTimestep()
{
    EV_DEBUG << "Triggering TraCI server simulation advance to t=" << simTime() << endl;

    uint32_t targetTime = getCurrentTimeMs();

    if (targetTime > round(connectAt.dbl() * 1000)) {
        TraCIBuffer buf = queryTraCI(CMD_SIMSTEP2, TraCIBuffer() << targetTime);

        uint32_t count;
        buf >> count;
        EV_DEBUG << "Getting " << count << " subscription results" << endl;
        for (uint32_t i = 0; i < count; ++i) {
            processSubcriptionResult(buf);
        }
    }

    if (!autoShutdownTriggered)
        scheduleAt(simTime() + updateInterval, executeOneTimestepTrigger);
}

std::string TraCIScenarioManager::genericGetString(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId)
{
    uint8_t resultTypeId = TYPE_STRING;
    std::string res;

    TraCIBuffer buf = queryTraCI(commandId, TraCIBuffer() << variableId << objectId);

    uint8_t cmdLength;
    buf >> cmdLength;
    if (cmdLength == 0) {
        uint32_t cmdLengthX;
        buf >> cmdLengthX;
    }
    uint8_t commandId_r;
    buf >> commandId_r;
    ASSERT(commandId_r == responseId);
    uint8_t varId;
    buf >> varId;
    ASSERT(varId == variableId);
    std::string objectId_r;
    buf >> objectId_r;
    ASSERT(objectId_r == objectId);
    uint8_t resType_r;
    buf >> resType_r;
    ASSERT(resType_r == resultTypeId);
    buf >> res;

    ASSERT(buf.eof());

    return res;
}

Coord TraCIScenarioManager::genericGetCoord(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId)
{
    uint8_t resultTypeId = POSITION_2D;
    double x;
    double y;

    TraCIBuffer buf = queryTraCI(commandId, TraCIBuffer() << variableId << objectId);

    uint8_t cmdLength;
    buf >> cmdLength;
    if (cmdLength == 0) {
        uint32_t cmdLengthX;
        buf >> cmdLengthX;
    }
    uint8_t commandId_r;
    buf >> commandId_r;
    ASSERT(commandId_r == responseId);
    uint8_t varId;
    buf >> varId;
    ASSERT(varId == variableId);
    std::string objectId_r;
    buf >> objectId_r;
    ASSERT(objectId_r == objectId);
    uint8_t resType_r;
    buf >> resType_r;
    ASSERT(resType_r == resultTypeId);
    buf >> x;
    buf >> y;

    ASSERT(buf.eof());

    return traci2omnet(TraCICoord(x, y));
}

double TraCIScenarioManager::genericGetDouble(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId)
{
    uint8_t resultTypeId = TYPE_DOUBLE;
    double res;

    TraCIBuffer buf = queryTraCI(commandId, TraCIBuffer() << variableId << objectId);

    uint8_t cmdLength;
    buf >> cmdLength;
    if (cmdLength == 0) {
        uint32_t cmdLengthX;
        buf >> cmdLengthX;
    }
    uint8_t commandId_r;
    buf >> commandId_r;
    ASSERT(commandId_r == responseId);
    uint8_t varId;
    buf >> varId;
    ASSERT(varId == variableId);
    std::string objectId_r;
    buf >> objectId_r;
    ASSERT(objectId_r == objectId);
    uint8_t resType_r;
    buf >> resType_r;
    ASSERT(resType_r == resultTypeId);
    buf >> res;

    ASSERT(buf.eof());

    return res;
}

std::list<std::string> TraCIScenarioManager::genericGetStringList(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId)
{
    uint8_t resultTypeId = TYPE_STRINGLIST;
    std::list<std::string> res;

    TraCIBuffer buf = queryTraCI(commandId, TraCIBuffer() << variableId << objectId);

    uint8_t cmdLength;
    buf >> cmdLength;
    if (cmdLength == 0) {
        uint32_t cmdLengthX;
        buf >> cmdLengthX;
    }
    uint8_t commandId_r;
    buf >> commandId_r;
    ASSERT(commandId_r == responseId);
    uint8_t varId;
    buf >> varId;
    ASSERT(varId == variableId);
    std::string objectId_r;
    buf >> objectId_r;
    ASSERT(objectId_r == objectId);
    uint8_t resType_r;
    buf >> resType_r;
    ASSERT(resType_r == resultTypeId);
    uint32_t count;
    buf >> count;
    for (uint32_t i = 0; i < count; i++) {
        std::string id;
        buf >> id;
        res.push_back(id);
    }

    ASSERT(buf.eof());

    return res;
}

std::list<Coord> TraCIScenarioManager::genericGetCoordList(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId)
{
    uint8_t resultTypeId = TYPE_POLYGON;
    std::list<Coord> res;

    TraCIBuffer buf = queryTraCI(commandId, TraCIBuffer() << variableId << objectId);

    uint8_t cmdLength;
    buf >> cmdLength;
    if (cmdLength == 0) {
        uint32_t cmdLengthX;
        buf >> cmdLengthX;
    }
    uint8_t commandId_r;
    buf >> commandId_r;
    ASSERT(commandId_r == responseId);
    uint8_t varId;
    buf >> varId;
    ASSERT(varId == variableId);
    std::string objectId_r;
    buf >> objectId_r;
    ASSERT(objectId_r == objectId);
    uint8_t resType_r;
    buf >> resType_r;
    ASSERT(resType_r == resultTypeId);
    uint8_t count;
    buf >> count;
    for (uint32_t i = 0; i < count; i++) {
        double x;
        buf >> x;
        double y;
        buf >> y;
        Coord pos = traci2omnet(TraCICoord(x, y));
        res.push_back(pos);
    }

    ASSERT(buf.eof());

    return res;
}

Coord TraCIScenarioManager::traci2omnet(TraCICoord coord) const
{
    return Coord(coord.x - netbounds1.x + margin, (netbounds2.y - netbounds1.y) - (coord.y - netbounds1.y) + margin);
}

TraCIScenarioManager::TraCICoord TraCIScenarioManager::omnet2traci(Coord coord) const
{
    return TraCICoord(coord.x + netbounds1.x - margin, (netbounds2.y - netbounds1.y) - (coord.y - netbounds1.y) + margin);
}

double TraCIScenarioManager::traci2omnetAngle(double angle) const
{
    // rotate angle so 0 is east (in TraCI's angle interpretation 0 is south)
    angle = angle - 90;

    // convert to rad
    angle = angle * M_PI / 180.0;

    // normalize angle to -M_PI <= angle < M_PI
    while (angle < -M_PI)
        angle += 2 * M_PI;
    while (angle >= M_PI)
        angle -= 2 * M_PI;

    return angle;
}

double TraCIScenarioManager::omnet2traciAngle(double angle) const
{
    // convert to degrees
    angle = angle * 180 / M_PI;

    // rotate angle so 0 is south (in OMNeT++'s angle interpretation 0 is east)
    angle = angle + 90;

    // normalize angle to -180 <= angle < 180
    while (angle < -180)
        angle += 360;
    while (angle >= 180)
        angle -= 360;

    return angle;
}

void TraCIScenarioManager::subscribeToVehicleVariables(std::string vehicleId)
{
    // subscribe to some attributes of the vehicle
    uint32_t beginTime = 0;
    uint32_t endTime = 0x7FFFFFFF;
    std::string objectId = vehicleId;
    uint8_t variableNumber = 5;
    uint8_t variable1 = VAR_POSITION;
    uint8_t variable2 = VAR_ROAD_ID;
    uint8_t variable3 = VAR_SPEED;
    uint8_t variable4 = VAR_ANGLE;
    uint8_t variable5 = VAR_SIGNALS;

    TraCIBuffer buf = queryTraCI(CMD_SUBSCRIBE_VEHICLE_VARIABLE, TraCIBuffer() << beginTime << endTime << objectId << variableNumber << variable1 << variable2 << variable3 << variable4 << variable5);
    processSubcriptionResult(buf);
    ASSERT(buf.eof());
}

void TraCIScenarioManager::unsubscribeFromVehicleVariables(std::string vehicleId)
{
    // subscribe to some attributes of the vehicle
    uint32_t beginTime = 0;
    uint32_t endTime = 0x7FFFFFFF;
    std::string objectId = vehicleId;
    uint8_t variableNumber = 0;

    TraCIBuffer buf = queryTraCI(CMD_SUBSCRIBE_VEHICLE_VARIABLE, TraCIBuffer() << beginTime << endTime << objectId << variableNumber);
    ASSERT(buf.eof());
}

void TraCIScenarioManager::processSimSubscription(std::string objectId, TraCIBuffer& buf)
{
    uint8_t variableNumber_resp;
    buf >> variableNumber_resp;
    for (uint8_t j = 0; j < variableNumber_resp; ++j) {
        uint8_t variable1_resp;
        buf >> variable1_resp;
        uint8_t isokay;
        buf >> isokay;
        if (isokay != RTYPE_OK) {
            uint8_t varType;
            buf >> varType;
            ASSERT(varType == TYPE_STRING);
            std::string description;
            buf >> description;
            if (isokay == RTYPE_NOTIMPLEMENTED)
                throw cRuntimeError("TraCI server reported subscribing to variable 0x%2x not implemented (\"%s\"). Might need newer version.", variable1_resp, description.c_str());
            throw cRuntimeError("TraCI server reported error subscribing to variable 0x%2x (\"%s\").", variable1_resp, description.c_str());
        }

        if (variable1_resp == VAR_DEPARTED_VEHICLES_IDS) {
            uint8_t varType;
            buf >> varType;
            ASSERT(varType == TYPE_STRINGLIST);
            uint32_t count;
            buf >> count;
            EV_DEBUG << "TraCI reports " << count << " departed vehicles." << endl;
            for (uint32_t i = 0; i < count; ++i) {
                std::string idstring;
                buf >> idstring;
                // adding modules is handled on the fly when entering/leaving the ROI
            }

            activeVehicleCount += count;
        }
        else if (variable1_resp == VAR_ARRIVED_VEHICLES_IDS) {
            uint8_t varType;
            buf >> varType;
            ASSERT(varType == TYPE_STRINGLIST);
            uint32_t count;
            buf >> count;
            EV_DEBUG << "TraCI reports " << count << " arrived vehicles." << endl;
            for (uint32_t i = 0; i < count; ++i) {
                std::string idstring;
                buf >> idstring;

                if (subscribedVehicles.find(idstring) != subscribedVehicles.end()) {
                    subscribedVehicles.erase(idstring);
                    unsubscribeFromVehicleVariables(idstring);
                }

                // check if this object has been deleted already (e.g. because it was outside the ROI)
                cModule *mod = getManagedModule(idstring);
                if (mod)
                    deleteModule(idstring);

                if (unEquippedHosts.find(idstring) != unEquippedHosts.end()) {
                    unEquippedHosts.erase(idstring);
                }
            }

            if ((count > 0) && (count >= activeVehicleCount) && autoShutdown)
                autoShutdownTriggered = true;
            activeVehicleCount -= count;
        }
        else if (variable1_resp == VAR_TIME_STEP) {
            uint8_t varType;
            buf >> varType;
            ASSERT(varType == TYPE_INTEGER);
            uint32_t serverTimestep;
            buf >> serverTimestep;
            EV_DEBUG << "TraCI reports current time step as " << serverTimestep << "ms." << endl;
            uint32_t omnetTimestep = getCurrentTimeMs();
            ASSERT(omnetTimestep == serverTimestep);
        }
        else {
            throw cRuntimeError("Received unhandled sim subscription result");
        }
    }
}

void TraCIScenarioManager::processVehicleSubscription(std::string objectId, TraCIBuffer& buf)
{
    bool isSubscribed = (subscribedVehicles.find(objectId) != subscribedVehicles.end());
    double px = 1.0 / 0.0;
    double py = 1.0 / 0.0;
    std::string edge;
    double speed = 1.0 / 0.0;
    double angle_traci = 1.0 / 0.0;
    int signals;
    int numRead = 0;

    uint8_t variableNumber_resp;
    buf >> variableNumber_resp;
    for (uint8_t j = 0; j < variableNumber_resp; ++j) {
        uint8_t variable1_resp;
        buf >> variable1_resp;
        uint8_t isokay;
        buf >> isokay;
        if (isokay != RTYPE_OK) {
            uint8_t varType;
            buf >> varType;
            ASSERT(varType == TYPE_STRING);
            std::string errormsg;
            buf >> errormsg;
            if (isSubscribed) {
                if (isokay == RTYPE_NOTIMPLEMENTED)
                    throw cRuntimeError("TraCI server reported subscribing to vehicle variable 0x%2x not implemented (\"%s\"). Might need newer version.", variable1_resp, errormsg.c_str());
                throw cRuntimeError("TraCI server reported error subscribing to vehicle variable 0x%2x (\"%s\").", variable1_resp, errormsg.c_str());
            }
        }
        else if (variable1_resp == ID_LIST) {
            uint8_t varType;
            buf >> varType;
            ASSERT(varType == TYPE_STRINGLIST);
            uint32_t count;
            buf >> count;
            EV_DEBUG << "TraCI reports " << count << " active vehicles." << endl;
            ASSERT(count == activeVehicleCount);
            std::set<std::string> drivingVehicles;
            for (uint32_t i = 0; i < count; ++i) {
                std::string idstring;
                buf >> idstring;
                drivingVehicles.insert(idstring);
            }

            // check for vehicles that need subscribing to
            std::set<std::string> needSubscribe;
            std::set_difference(drivingVehicles.begin(), drivingVehicles.end(), subscribedVehicles.begin(), subscribedVehicles.end(), std::inserter(needSubscribe, needSubscribe.begin()));
            for (std::set<std::string>::const_iterator i = needSubscribe.begin(); i != needSubscribe.end(); ++i) {
                subscribedVehicles.insert(*i);
                subscribeToVehicleVariables(*i);
            }

            // check for vehicles that need unsubscribing from
            std::set<std::string> needUnsubscribe;
            std::set_difference(subscribedVehicles.begin(), subscribedVehicles.end(), drivingVehicles.begin(), drivingVehicles.end(), std::inserter(needUnsubscribe, needUnsubscribe.begin()));
            for (std::set<std::string>::const_iterator i = needUnsubscribe.begin(); i != needUnsubscribe.end(); ++i) {
                subscribedVehicles.erase(*i);
                unsubscribeFromVehicleVariables(*i);
            }
        }
        else if (variable1_resp == VAR_POSITION) {
            uint8_t varType;
            buf >> varType;
            ASSERT(varType == POSITION_2D);
            buf >> px;
            buf >> py;
            numRead++;
        }
        else if (variable1_resp == VAR_ROAD_ID) {
            uint8_t varType;
            buf >> varType;
            ASSERT(varType == TYPE_STRING);
            buf >> edge;
            numRead++;
        }
        else if (variable1_resp == VAR_SPEED) {
            uint8_t varType;
            buf >> varType;
            ASSERT(varType == TYPE_DOUBLE);
            buf >> speed;
            numRead++;
        }
        else if (variable1_resp == VAR_ANGLE) {
            uint8_t varType;
            buf >> varType;
            ASSERT(varType == TYPE_DOUBLE);
            buf >> angle_traci;
            numRead++;
        }
        else if (variable1_resp == VAR_SIGNALS) {
            uint8_t varType;
            buf >> varType;
            ASSERT(varType == TYPE_INTEGER);
            buf >> signals;
            numRead++;
        }
        else {
            throw cRuntimeError("Received unhandled vehicle subscription result");
        }
    }

    // bail out if we didn't want to receive these subscription results
    if (!isSubscribed)
        return;

    // make sure we got updates for all attributes
    if (numRead != 5)
        return;

    Coord p = traci2omnet(TraCICoord(px, py));
    if ((p.x < 0) || (p.y < 0))
        throw cRuntimeError("received bad node position (%.2f, %.2f), translated to (%.2f, %.2f)", px, py, p.x, p.y);

    double angle = traci2omnetAngle(angle_traci);

    cModule *mod = getManagedModule(objectId);

    // is it in the ROI?
    bool inRoi = isInRegionOfInterest(TraCICoord(px, py), edge, speed, angle);
    if (!inRoi) {
        if (mod) {
            deleteModule(objectId);
            EV_DEBUG << "Vehicle #" << objectId << " left region of interest" << endl;
        }
        else if (unEquippedHosts.find(objectId) != unEquippedHosts.end()) {
            unEquippedHosts.erase(objectId);
            EV_DEBUG << "Vehicle (unequipped) # " << objectId << " left region of interest" << endl;
        }
        return;
    }

    if (isModuleUnequipped(objectId)) {
        return;
    }

    if (!mod) {
        // no such module - need to create
        addModule(objectId, moduleType, moduleName, moduleDisplayString, p, edge, speed, angle);
        EV_DEBUG << "Added vehicle #" << objectId << endl;
    }
    else {
        // module existed - update position
        for (cModule::SubmoduleIterator iter(mod); !iter.end(); iter++) {
            cModule *submod = iter();
            TraCIMobility *mm = dynamic_cast<TraCIMobility *>(submod);
            if (!mm)
                continue;
            EV_DEBUG << "module " << objectId << " moving to " << p.x << "," << p.y << endl;
            mm->nextPosition(p, edge, speed, angle);
        }
    }
}

void TraCIScenarioManager::processSubcriptionResult(TraCIBuffer& buf)
{
    uint8_t cmdLength_resp;
    buf >> cmdLength_resp;
    uint32_t cmdLengthExt_resp;
    buf >> cmdLengthExt_resp;
    uint8_t commandId_resp;
    buf >> commandId_resp;
    std::string objectId_resp;
    buf >> objectId_resp;

    if (commandId_resp == RESPONSE_SUBSCRIBE_VEHICLE_VARIABLE)
        processVehicleSubscription(objectId_resp, buf);
    else if (commandId_resp == RESPONSE_SUBSCRIBE_SIM_VARIABLE)
        processSimSubscription(objectId_resp, buf);
    else {
        throw cRuntimeError("Received unhandled subscription result");
    }
}

template<> void TraCIScenarioManager::TraCIBuffer::write(std::string inv)
{
    uint32_t length = inv.length();
    write<uint32_t>(length);
    for (size_t i = 0; i < length; ++i)
        write<char>(inv[i]);
}

template<> std::string TraCIScenarioManager::TraCIBuffer::read()
{
    uint32_t length = read<uint32_t>();
    if (length == 0)
        return std::string();
    char obuf[length + 1];

    for (size_t i = 0; i < length; ++i)
        read<char>(obuf[i]);
    obuf[length] = 0;

    return std::string(obuf, length);
}

} // namespace inet

#endif    // WITH_TRACI

