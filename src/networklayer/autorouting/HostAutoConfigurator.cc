/*
 * HostAutoConfigurator - automatically assigns IP addresses and sets up routing table
 * Copyright (C) 2009 Christoph Sommer <christoph.sommer@informatik.uni-erlangen.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <stdexcept>
#include <algorithm>

#include "networklayer/autorouting/HostAutoConfigurator.h"

#include "IPAddressResolver.h"
#include "IPv4InterfaceData.h"
#include "IRoutingTable.h"
#include "IInterfaceTable.h"
#include "IPAddress.h"

Define_Module(HostAutoConfigurator);

void HostAutoConfigurator::initialize(int stage) {
	cSimpleModule::initialize(stage);

	if (stage == 0) {
		debug = par("debug").boolValue();
	}
	else if (stage == 2) {
		setupNetworkLayer();
	}
}

void HostAutoConfigurator::finish() {
}

void HostAutoConfigurator::handleMessage(cMessage* apMsg) {
}

void HostAutoConfigurator::handleSelfMsg(cMessage* apMsg) {
}

namespace {
	void addToMcastGroup(InterfaceEntry* ie, IRoutingTable* routingTable, const IPAddress& mcastGroup) {
		IPv4InterfaceData::IPAddressVector mcg = ie->ipv4Data()->getMulticastGroups();
		if (std::find(mcg.begin(), mcg.end(), mcastGroup) == mcg.end()) mcg.push_back(mcastGroup);
		ie->ipv4Data()->setMulticastGroups(mcg);

		IPRoute* re = new IPRoute(); //TODO: add @c delete to destructor
		re->setHost(mcastGroup);
		re->setNetmask(IPAddress::ALLONES_ADDRESS); // TODO: can't set this to none?
		re->setGateway(IPAddress()); // none
		re->setInterface(ie);
		re->setType(IPRoute::DIRECT);
		re->setSource(IPRoute::MANUAL);
		re->setMetric(1);
		routingTable->addRoute(re);
	}
}

void HostAutoConfigurator::setupNetworkLayer()
{
	EV << "host auto configuration started" << std::endl;

	std::string interfaces = par("interfaces").stringValue();
	IPAddress addressBase = IPAddress(par("addressBase").stringValue());
	IPAddress netmask = IPAddress(par("netmask").stringValue());
	std::string mcastGroups = par("mcastGroups").stringValue();
	IPAddress myAddress = IPAddress(addressBase.getInt() + uint32(getParentModule()->getId()));

	// get our host module
	cModule* host = getParentModule();
	if (!host) throw std::runtime_error("No parent module found");

	// get our routing table
	IRoutingTable* routingTable = IPAddressResolver().routingTableOf(host);
	if (!routingTable) throw std::runtime_error("No routing table found");

	// get our interface table
	IInterfaceTable *ift = IPAddressResolver().interfaceTableOf(host);
	if (!ift) throw std::runtime_error("No interface table found");

	// look at all interface table entries
	cStringTokenizer interfaceTokenizer(interfaces.c_str());
	const char *ifname;
	while ((ifname = interfaceTokenizer.nextToken()) != NULL) {
		InterfaceEntry* ie = ift->getInterfaceByName(ifname);
		if (!ie) throw std::runtime_error("No such interface");

		// assign IP Address to all connected interfaces
		if (ie->isLoopback()) {
			EV << "interface " << ifname << " skipped (is loopback)" << std::endl;
			continue;
		}

		EV << "interface " << ifname << " gets " << myAddress.str() << "/" << netmask.str() << std::endl;

		ie->ipv4Data()->setIPAddress(myAddress);
		ie->ipv4Data()->setNetmask(netmask);
		ie->setBroadcast(true);

		// associate interface with default multicast groups
		addToMcastGroup(ie, routingTable, IPAddress::ALL_HOSTS_MCAST);
		addToMcastGroup(ie, routingTable, IPAddress::ALL_ROUTERS_MCAST);

		// associate interface with specified multicast groups
		cStringTokenizer interfaceTokenizer(mcastGroups.c_str());
		const char *mcastGroup_s;
		while ((mcastGroup_s = interfaceTokenizer.nextToken()) != NULL) {
			IPAddress mcastGroup(mcastGroup_s);
			addToMcastGroup(ie, routingTable, mcastGroup);
		}
	}
}

