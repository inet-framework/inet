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

#include "applications/traci/TraCIDemo.h"

#include "NotificationBoard.h"
#include "UDPSocket.h"

Define_Module(TraCIDemo);

void TraCIDemo::initialize(int stage) {
	BasicModule::initialize(stage);
	if (stage == 0) {
		debug = par("debug");

		traci = TraCIMobilityAccess().get();
		sentMessage = false;

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

void TraCIDemo::handleMessage(cMessage* msg) {
	if (msg->isSelfMessage()) {
		handleSelfMsg(msg);
	} else {
		handleLowerMsg(msg);
	}
}

void TraCIDemo::handleSelfMsg(cMessage* msg) {
}

void TraCIDemo::handleLowerMsg(cMessage* msg) {
	if (!sentMessage) sendMessage();
	delete msg;
}

void TraCIDemo::receiveChangeNotification(int category, const cPolymorphic *details) {
	Enter_Method_Silent();

	if (category == NF_HOSTPOSITION_UPDATED) {
		handlePositionUpdate();
	}
}

void TraCIDemo::sendMessage() {
	sentMessage = true;

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

void TraCIDemo::handlePositionUpdate() {
	if (traci->getPosition().x < 7350) {
		if (!sentMessage) sendMessage();
	}
}
