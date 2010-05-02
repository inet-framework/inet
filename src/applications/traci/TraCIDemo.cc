/*
 *  Copyright (C) 2010 Christoph Sommer <christoph.sommer@informatik.uni-erlangen.de>
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

#include <algorithm>
#include <numeric>

#include "applications/traci/TraCIDemo.h"
#include "NotificationBoard.h"

Define_Module(TraCIDemo);

TraCIDemo::~TraCIDemo() {
}

void TraCIDemo::initialize(int aStage) {

	BasicModule::initialize(aStage);

	if (0 == aStage) {
		debug = par("debug");

		traci = TraCIMobilityAccess().get();
		triggeredFlooding = false;

		NotificationBoard* nb = NotificationBoardAccess().get();
		nb->subscribe(this, NF_HOSTPOSITION_UPDATED);

		setupLowerLayer();
	}
}

void TraCIDemo::setupLowerLayer() {
	cMessage *msg = new cMessage("UDP_C_BIND", UDP_C_BIND);
	UDPControlInfo *ctrl = new UDPControlInfo();
	ctrl->setSrcPort(12345);
	ctrl->setSockId(UDPSocket::generateSocketId());
	msg->setControlInfo(ctrl);
	send(msg, "udp$o");
}

void TraCIDemo::finish() {
}

void TraCIDemo::receiveChangeNotification(int category, const cPolymorphic *details) {
	Enter_Method("receiveChangeNotification()");

	if (category == NF_HOSTPOSITION_UPDATED) {
		handlePositionUpdate();
	} else {
		error("should only be subscribed to NF_HOSTPOSITION_UPDATED, but received notification of category %d", category);
	}
}

void TraCIDemo::handleMessage(cMessage* apMsg) {
	if (apMsg->isSelfMessage()) {
		handleSelfMsg(apMsg);
	} else {
		handleLowerMsg(apMsg);
	}
}

void TraCIDemo::handleSelfMsg(cMessage* apMsg) {
}

void TraCIDemo::handleLowerMsg(cMessage* apMsg) {
	if (cPacket* m = dynamic_cast<cPacket*>(apMsg)) {
		sendMessage();
	}

	delete apMsg;
}

void TraCIDemo::handlePositionUpdate() {
	if ((traci->getPosition().x < 7350) && (!triggeredFlooding)) {
		triggeredFlooding = true;

		sendMessage();
	}
}

void TraCIDemo::sendMessage() {
	cPacket* newMessage = new cPacket();

	newMessage->setKind(UDP_C_DATA);
	UDPControlInfo *ctrl = new UDPControlInfo();
	ctrl->setSrcPort(12345);
	ctrl->setDestAddr(IPAddress::ALL_HOSTS_MCAST);
	ctrl->setDestPort(12345);
	delete(newMessage->removeControlInfo());
	newMessage->setControlInfo(ctrl);

	sendDelayed(newMessage, 0.010, "udp$o");
}

