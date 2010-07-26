/*
 *  Copyright (C) 2009 Christoph Sommer <christoph.sommer@informatik.uni-erlangen.de>
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

#include <iostream>
#include <fstream>
#include <iomanip>

#include "applications/traci/Test_TraCI.h"
#include "NotificationBoard.h"

Define_Module(Test_TraCI);

void Test_TraCI::initialize(int aStage) {
	cSimpleModule::initialize(aStage);
	mobility = dynamic_cast<TraCIMobility*>(getParentModule()->getSubmodule("mobility"));
	if (mobility == 0) error("Could not find mobility module of type TraCIMobility");

	if (aStage == 0) {
		debug = par("debug");

		visitedEdges.clear();
		hasStopped = false;

		if (!Test_TraCI::clearedLog) {
			Test_TraCI::clearedLog = true;
			std::ofstream logfile;
			logfile.open("Test_TraCI.log", std::ios::out);
			logfile.close();
		}

		NotificationBoard* nb = NotificationBoardAccess().get();
		nb->subscribe(this, NF_HOSTPOSITION_UPDATED);

		cXMLElement* commands_root_xml = par("commands").xmlValue();
		cXMLElementList commands_xml = commands_root_xml->getChildren();
		simtime_t lastTime = simTime();
		for (cXMLElementList::iterator iter=commands_xml.begin(); iter != commands_xml.end(); iter++) {
			const cXMLElement* e = *iter;

			const char* time_s = e->getAttribute("t");
			const char* dtime_s = e->getAttribute("dt");
			if (!time_s && !dtime_s) error("command is missing attribute \"t\" and \"dt\". Need exactly one.");
			if (time_s && dtime_s) error("command has both attribute \"t\" and \"dt\". Need exactly one.");
			simtime_t time = simTime();
			if (time_s) {
				time = parseOrBail<double>(e, "t");
			}
			else {
				time = lastTime + parseOrBail<double>(e, "dt");
			}

			if (time < lastTime) error("commands not in ascending chronological order or scheduled prior to node creation");
			lastTime = time;

			if (debug) {
				ev << "queueing for t=" << time << ":" << std::endl;
				e->debugDump();
			}

			commands.push_back(e);
			scheduleAt(time, new cMessage("command"));
		}
	}
}


void Test_TraCI::executeCommand(const cXMLElement* e) {
	const char* name_s = e->getTagName(); if (!name_s) error("command name not found");
	std::string name = name_s;

	if (debug) {
		ev << "executing:" << std::endl;
		e->debugDump();
	}

	if (name == "CMD_SETMAXSPEED") {
		float maxspeed = parseOrBail<float>(e, "maxspeed");
		mobility->commandSetMaximumSpeed(maxspeed);
	}
	else if (name == "CMD_CHANGEROUTE") {
		std::string roadid = parseOrBail<std::string>(e, "roadid");
		double traveltime = parseOrBail<double>(e, "traveltime");
		mobility->commandChangeRoute(roadid, traveltime);
	}
	else if (name == "CMD_DISTANCEREQUEST") {
		float p1x = parseOrBail<float>(e, "p1x");
		float p1y = parseOrBail<float>(e, "p1y");
		float p2x = parseOrBail<float>(e, "p2x");
		float p2y = parseOrBail<float>(e, "p2y");
		bool drivingDistance = e->getAttribute("drivingDistance");;
		float result = mobility->commandDistanceRequest(Coord(p1x, p1y), Coord(p2x, p2y), drivingDistance);

		std::ofstream logfile;
		logfile.open ("Test_TraCI.log", std::ios::out | std::ios::app);
		logfile << this->getFullPath() << "\t" << "commandDistanceRequest" << "\t" << std::setiosflags(std::ios::fixed) << std::setprecision(1) << result << std::endl;
		logfile.close();
	}
	else if (name == "CMD_STOP") {
		std::string roadid = parseOrBail<std::string>(e, "roadid");
		float pos = parseOrBail<float>(e, "pos");
		uint8_t laneid = parseOrBail<uint8_t>(e, "laneid");
		float radius = parseOrBail<float>(e, "radius");
		double waittime = parseOrBail<double>(e, "waittime");

		mobility->commandStopNode(roadid, pos, laneid, radius, waittime);
	}
	else {
		error("command \"%s\" unknown", name.c_str());
	}
}


void Test_TraCI::finish() {
	std::ofstream logfile;
	logfile.open ("Test_TraCI.log", std::ios::out | std::ios::app);

	std::string visitedEdges_s;
	for (std::set<std::string>::const_iterator i = visitedEdges.begin(); i != visitedEdges.end(); i++) {
		if (i != visitedEdges.begin()) visitedEdges_s += " ";
		visitedEdges_s += *i;
	}

	logfile << this->getFullPath() << "\t" << "visitedEdges" << "\t" << visitedEdges_s << std::endl;

	recordScalar("hasStopped", hasStopped);
	logfile << this->getFullPath() << "\t" << "hasStopped" << "\t" << hasStopped << std::endl;

	logfile.close();
}


void Test_TraCI::handleMessage(cMessage* apMsg) {
	if (!apMsg->isSelfMessage()) {
		error("This module only handles self messages");
		return;
	}

	if (commands.size() < 1) {
		error("Ran out of commands");
		return;
	}

	const cXMLElement* e = commands.front();
	commands.pop_front();
	executeCommand(e);
}


void Test_TraCI::receiveChangeNotification(int category, const cPolymorphic *details) {
	Enter_Method("receiveChangeNotification()");

	if (category != NF_HOSTPOSITION_UPDATED) error("Test_TraCI should only be subscribed to NF_HOSTPOSITION_UPDATED, but received notification of category %d", category);

	try {
		double speed = mobility->getSpeed();

		std::string roadId = mobility->getRoadId();
		if ((roadId.length() > 0) && (roadId[0] != ':')) visitedEdges.insert(roadId);
		if (speed < 0.001) hasStopped = true;
	}
	catch (std::runtime_error e) {
		// We didn't already receive movement commands from TraCI
	}
}


template<> double Test_TraCI::extract(cDynamicExpression& o) {
	return o.doubleValue(this);
}
template<> float Test_TraCI::extract(cDynamicExpression& o) {
	return o.doubleValue(this);
}
template<> uint8_t Test_TraCI::extract(cDynamicExpression& o) {
	return o.longValue(this);
}
template<> std::string Test_TraCI::extract(cDynamicExpression& o) {
	return o.stringValue(this);
}
template<> std::string Test_TraCI::parseOrBail(const cXMLElement* xmlElement, std::string name) {
	try {
		const char* value_s = xmlElement->getAttribute(name.c_str());
		if (!value_s) throw new std::runtime_error("missing attribute");
		return value_s;
	}
	catch (std::runtime_error e) {
		error((std::string("command parse error for attribute \"") + name + "\": " + e.what()).c_str());
		throw e;
	}
}

bool Test_TraCI::clearedLog = false;

