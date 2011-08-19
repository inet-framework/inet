/*
 *  Copyright (C) 2009 Christoph Sommer <christoph.sommer@informatik.uni-erlangen.de>
 *
 *  Documentation for these modules is at http://veins.car2x.org/
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "applications/traci/TraCITestApp.h"
#include "NotificationBoard.h"
#include <cmath>

Define_Module(TraCITestApp);

void TraCITestApp::initialize(int stage) {
	cSimpleModule::initialize(stage);
	if (stage == 0) {
		debug = par("debug");
		testNumber = par("testNumber");
		NotificationBoard* nb = NotificationBoardAccess().get();
		nb->subscribe(this, NF_HOSTPOSITION_UPDATED);
		traci = TraCIMobilityAccess().get();

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


void TraCITestApp::receiveChangeNotification(int category, const cPolymorphic *details) {
	Enter_Method_Silent();

	if (category == NF_HOSTPOSITION_UPDATED) {
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
}

void TraCITestApp::handlePositionUpdate() {
	const simtime_t t = simTime();
	const std::string roadId = traci->getRoadId();
	visitedEdges.insert(roadId);

	if (testNumber == 0) {
		if (t == 9) {
			assertTrue("vehicle is driving", traci->getSpeed() > 25);
		}
		if (t == 10) {
			traci->commandSetSpeedMode(0x00);
			traci->commandSetSpeed(0);
		}
		if (t == 11) {
			assertClose("vehicle has stopped", traci->getSpeed(), 0.0);
		}
	}

	if (testNumber == 1) {
		if (t == 1) {
			/*
			double target = traci->commandDistanceRequest(Coord(25,7030), Coord(883,6980), false);
			double actual = 859.456;
			std::cout.precision(10);
			std::cout << target << std::endl;
			std::cout << actual << std::endl;
			std::cout << fabs(target - actual) << std::endl;
			*/
			assertClose("commandDistanceRequest (air)", traci->commandDistanceRequest(Coord(25,7030), Coord(883,6980), false), 859.4556417);
			assertClose("commandDistanceRequest (driving)", traci->commandDistanceRequest(Coord(25,7030), Coord(883,6980), true), 847.5505384);
		}
	}

	if (testNumber == 2) {
		if (t == 1) {
			traci->commandChangeRoute("42", 9999);
			traci->commandChangeRoute("43", 9999);
		}
		if (t == 30) {
			assertTrue("vehicle avoided 42", visitedEdges.find("42") == visitedEdges.end());
			assertTrue("vehicle avoided 43", visitedEdges.find("43") == visitedEdges.end());
			assertTrue("vehicle took 44", visitedEdges.find("44") != visitedEdges.end());
		}
	}

	if (testNumber == 3) {
		if (t == 1) {
			traci->commandChangeRoute("42", 9999);
			traci->commandChangeRoute("43", 9999);
		}
		if (t == 3) {
			traci->commandChangeRoute("42", -1);
			traci->commandChangeRoute("44", 9999);
		}
		if (t == 30) {
			assertTrue("vehicle took 42", visitedEdges.find("42") != visitedEdges.end());
			assertTrue("vehicle avoided 43", visitedEdges.find("43") == visitedEdges.end());
			assertTrue("vehicle avoided 44", visitedEdges.find("44") == visitedEdges.end());
		}
	}

	if (testNumber == 4) {
		if (t == 1) {
			traci->commandStopNode("43", 20, 0, 10, 30);
		}
		if (t == 30) {
			assertTrue("vehicle is at 43", roadId == "43");
			assertClose("vehicle is stopped", traci->getSpeed(), 0.0);
		}
	}
}

