//
// Copyright (C) 2006-2011 Christoph Sommer <christoph.sommer@uibk.ac.at>
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

#include "applications/traci/TraCITestApp.h"
#include "NotificationBoard.h"
#include <cmath>

Define_Module(TraCITestApp);

void TraCITestApp::initialize(int stage) {
    cSimpleModule::initialize(stage);
    if (stage == 0) {
        debug = par("debug");
        testNumber = par("testNumber");

        mobilityStateChangedSignal = registerSignal("mobilityStateChanged");
        traci = TraCIMobilityAccess().get();
        traci->subscribe(mobilityStateChangedSignal, this);

        visitedEdges.clear();
        hasStopped = false;

        if (debug) std::cout << "TraCITestApp initialized with testNumber=" << testNumber << std::endl;
    }
}

void TraCITestApp::finish() {
}

void TraCITestApp::handleSelfMsg(cMessage *msg) {
}

void TraCITestApp::handleLowerMsg(cMessage* msg) {
    delete msg;
}

void TraCITestApp::handleMessage(cMessage* msg) {
    if (msg->isSelfMessage()) {
        handleSelfMsg(msg);
    } else {
        handleLowerMsg(msg);
    }
}


void TraCITestApp::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj) {
    if (signalID == mobilityStateChangedSignal) {
        handlePositionUpdate();
    }
}

namespace {
    void assertTrue(std::string msg, bool b) {
        std::cout << (b?"Passed":"FAILED") << ": " << msg << std::endl;
    }

    template<class T> void assertClose(std::string msg, T target, T actual) {
        assertTrue(msg, std::fabs(target - actual) <= 0.0000001);
    }
    template<class T> void assertEqual(std::string msg, T target, T actual) {
        assertTrue(msg, target == actual);
    }
    void assertEqual(std::string msg, std::string target, std::string actual) {
        assertTrue(msg, target == actual);
    }
}

void TraCITestApp::handlePositionUpdate() {
    const simtime_t t = simTime();
    const std::string roadId = traci->getRoadId();
    visitedEdges.insert(roadId);

    int testCounter = 0;

    if (testNumber == testCounter++) {
        if (t == 9) {
            assertTrue("(commandSetSpeed) vehicle is driving", traci->getSpeed() > 25);
        }
        if (t == 10) {
            traci->commandSetSpeedMode(0x00);
            traci->commandSetSpeed(0);
        }
        if (t == 11) {
            assertClose("(commandSetSpeed) vehicle has stopped", 0.0, traci->getSpeed());
        }
    }

    if (testNumber == testCounter++) {
        if (t == 1) {
            traci->commandChangeRoute("42", 9999);
            traci->commandChangeRoute("43", 9999);
        }
        if (t == 30) {
            assertTrue("(commandChangeRoute, 9999) vehicle avoided 42", visitedEdges.find("42") == visitedEdges.end());
            assertTrue("(commandChangeRoute, 9999) vehicle avoided 43", visitedEdges.find("43") == visitedEdges.end());
            assertTrue("(commandChangeRoute, 9999) vehicle took 44", visitedEdges.find("44") != visitedEdges.end());
        }
    }

    if (testNumber == testCounter++) {
        if (t == 1) {
            traci->commandChangeRoute("42", 9999);
            traci->commandChangeRoute("43", 9999);
        }
        if (t == 3) {
            traci->commandChangeRoute("42", -1);
            traci->commandChangeRoute("44", 9999);
        }
        if (t == 30) {
            assertTrue("(commandChangeRoute, -1) vehicle took 42", visitedEdges.find("42") != visitedEdges.end());
            assertTrue("(commandChangeRoute, -1) vehicle avoided 43", visitedEdges.find("43") == visitedEdges.end());
            assertTrue("(commandChangeRoute, -1) vehicle avoided 44", visitedEdges.find("44") == visitedEdges.end());
        }
    }

    if (testNumber == testCounter++) {
        if (t == 1) {
            assertClose("(commandDistanceRequest, air)", 859.4556417, traci->commandDistanceRequest(Coord(25,7030), Coord(883,6980), false));
            assertClose("(commandDistanceRequest, driving)", 845.93, traci->commandDistanceRequest(Coord(25,7030), Coord(883,6980), true));
        }
    }

    if (testNumber == testCounter++) {
        if (t == 1) {
            traci->commandStopNode("43", 20, 0, 10, 30);
        }
        if (t == 30) {
            assertTrue("(commandStopNode) vehicle is at 43", roadId == "43");
            assertClose("(commandStopNode) vehicle is stopped", 0.0, traci->getSpeed());
        }
    }

    if (testNumber == testCounter++) {
        if (t == 1) {
            traci->getManager()->commandSetTrafficLightProgram("10", "myProgramRed");
        }
        if (t == 30) {
            assertTrue("(commandSetTrafficLightProgram) vehicle is at 31", roadId == "31");
            assertClose("(commandSetTrafficLightProgram) vehicle is stopped", 0.0, traci->getSpeed());
        }
    }

    if (testNumber == testCounter++) {
        if (t == 1) {
            traci->getManager()->commandSetTrafficLightPhaseIndex("10", 4);
        }
        if (t == 30) {
            assertTrue("(commandSetTrafficLightPhaseIndex) vehicle is at 31", roadId == "31");
            assertClose("(commandSetTrafficLightPhaseIndex) vehicle is stopped", 0.0, traci->getSpeed());
        }
    }

    if (testNumber == testCounter++) {
        if (t == 1) {
            std::list<std::string> polys = traci->commandGetPolygonIds();
            assertEqual("(commandGetPolygonIds) number is 1", polys.size(), (size_t)1);
            assertEqual("(commandGetPolygonIds) id is correct", *polys.begin(), "poly0");
            std::string typeId = traci->commandGetPolygonTypeId("poly0");
            assertEqual("(commandGetPolygonTypeId) typeId is correct", typeId, "type0");
            std::list<Coord> shape = traci->commandGetPolygonShape("poly0");
            assertClose("(commandGetPolygonShape) shape x coordinate is correct", 130.0, shape.begin()->x);
            assertClose("(commandGetPolygonShape) shape y coordinate is correct", 81.65, shape.begin()->y);
        }
    }

    if (testNumber == testCounter++) {
        if (t == 1) {
            std::list<Coord> shape1 = traci->commandGetPolygonShape("poly0");
            assertClose("(commandGetPolygonShape) shape x coordinate is correct", 130.0, shape1.begin()->x);
            assertClose("(commandGetPolygonShape) shape y coordinate is correct", 81.65, shape1.begin()->y);
            std::list<Coord> shape2 = shape1;
            shape2.begin()->x = 135;
            shape2.begin()->y = 85;
            traci->commandSetPolygonShape("poly0", shape2);
            std::list<Coord> shape3 = traci->commandGetPolygonShape("poly0");
            assertClose("(commandSetPolygonShape) shape x coordinate was changed", 135.0, shape3.begin()->x);
            assertClose("(commandSetPolygonShape) shape y coordinate was changed", 85.0, shape3.begin()->y);
        }
    }

    if (testNumber == testCounter++) {
        if (t == 30) {
            bool r = traci->getManager()->commandAddVehicle("testVehicle0", "vtype0", "route0", "25_0", 0, 70);
            assertTrue("(commandAddVehicle) command reports success", r);
        }
        if (t == 31) {
            std::map<std::string, cModule*>::const_iterator i = traci->getManager()->getManagedHosts().find("testVehicle0");
            bool r = (i != traci->getManager()->getManagedHosts().end());
            assertTrue("(commandAddVehicle) vehicle now driving", r);
            cModule* mod = i->second;
            TraCIMobility* traci2 = check_and_cast<TraCIMobility*>(findModuleWhereverInNode("mobility", mod));
            assertTrue("(commandAddVehicle) vehicle driving at speed", traci2->getSpeed() > 25);
        }
    }
}

