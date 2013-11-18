// Copyright (C) 2013 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
/**
 * @file PimSplitter.cc
 * @date 3.12.2011
 * @author Veronika Rybova, Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)
 * @brief File contains implementation of PIMSplitter.
 * @details Splitter is common for all PIM modes. It is used to resent all PIM messages to
 *  correct PIM mode module. It also does work which is same for all modes, e.g.
 *  it send Hello messages and it manages table of PIM interfaces.
 */

#include "IPv4Datagram.h"
#include "PIMSplitter.h"

using namespace std;

Define_Module(PIMSplitter);


/**
 * CREATE HELLO PACKET
 *
 * The method creates new PIM Hello Packet and sets all necessary info.
 *
 * @param iftID ID of interface to which the packet has to be sent
 * @return Return PIMHello message, which is ready to be sent.
 * @see PIMHello
 */
PIMHello* PIMSplitter::createHelloPkt(int iftID)
{
	PIMHello *msg = new PIMHello();
	msg->setName("PIMHello");

	IPv4ControlInfo *ctrl = new IPv4ControlInfo();
	IPv4Address ga1("224.0.0.13");
	ctrl->setDestAddr(ga1);
	//ctrl->setProtocol(IP_PROT_PIM);
	ctrl->setProtocol(103);
	ctrl->setTimeToLive(1);
	ctrl->setInterfaceId(iftID);
	msg->setControlInfo(ctrl);

	return msg;
}

/**
 * SEND HELLO PACKET
 *
 * The method goes through all PIM interfaces and sends Hello
 * packet to each of them. It also schedule next sending of
 * Hello packets (sets Hello Timer).
 *
 * @see createHelloPkt()
 */
void PIMSplitter::sendHelloPkt()
{
	EV << "PIM::sendHelloPkt" << endl;
	int intID;
	PIMHello* msg;

	// send to all PIM interfaces
	for (int i = 0; i < pimIft->getNumInterface(); i++)
	{
		intID = pimIft->getInterface(i)->getInterfaceID();
		msg = createHelloPkt(intID);
		send(msg, "transportOut");
	}

	// start Hello timer
	PIMTimer *timer = new PIMTimer("Hello");
	timer->setTimerKind(HelloTimer);
	scheduleAt(simTime() + HT, timer);
}

/**
 * PROCESS HELLO PACKET
 *
 * The method processes new coming Hello packet from any of its
 * neighbor. It reads info about neighbor from the packet and
 * tries to find neighbor in PimNeighborTable. If neighbor is
 * not in the table, method adds him and sets Neighbor Liveness Timer for
 * the record. If neighbor is already in PimNeighborTable it
 * refreshes Neighbor Liveness Timer
 *
 * @param msg Pointer to incoming Hello packet
 * @see PimNeighbor
 * @see PIMnlt
 */
void PIMSplitter::processHelloPkt(PIMPacket *msg)
{
    EV << "PIM::processHelloPkt" << endl;

    IPv4ControlInfo *ctrl = dynamic_cast<IPv4ControlInfo *>(msg->getControlInfo());
    PimNeighbor newEntry;
    PIMnlt *nlt;

    // get information about neighbor from Hello packet
    newEntry.setAddr(ctrl->getSrcAddr());
    newEntry.setInterfaceID(ctrl->getInterfaceId());
    newEntry.setInterfacePtr(ift->getInterfaceById(ctrl->getInterfaceId()));
    newEntry.setVersion(msg->getVersion());


    // new neighbor (it is not in PIM neighbor table)
    // insert new neighbor to table
    // set Neighbor Livness Timer
    if (!pimNbt->isInTable(newEntry))
    {
        nlt = new PIMnlt("NeighborLivenessTimer");
        nlt->setTimerKind(NeighborLivenessTimer);
        nlt->setNtId(pimNbt->getIdCounter());
        scheduleAt(simTime() + 3.5*HT, nlt);

        newEntry.setNlt(nlt);
        pimNbt->addNeighbor(newEntry);
        EV << "PimSplitter::New Entry was added: addr = " << newEntry.getAddr() << ", iftID = " << newEntry.getInterfaceID() << ", ver = " << newEntry.getVersion() << endl;
    }
    // neighbor is already in PIM neighbor table
    // refresh Neighbor Livness Timer
    else
    {
        nlt = pimNbt->findNeighbor(ctrl->getInterfaceId(), ctrl->getSrcAddr())->getNlt();
        cancelEvent(nlt);
        scheduleAt(simTime() + 3.5*HT, nlt);
    }

    delete msg;
}

/**
 * PROCESS NEIGHBOR LIVENESS TIMER
 *
 * The method process Neighbor Liveness Timer. After its expiration neighbor is removed
 * from PimNeighborTable.
 *
 * @param timer PIM Neighbor Liveness Timer.
 * @see PimNeighbor
 * @see PIMnlt()
 */
void PIMSplitter::processNLTimer(PIMTimer *timer)
{
	EV << "PIM::processNLTimer"<< endl;
	PIMnlt *nlt = check_and_cast <PIMnlt *> (timer);
	int id = nlt->getNtId();
	IPv4Address neighbor;

	// if neighbor exists store its IP address
	if (pimNbt->getNeighborsByID(id) != NULL)
		neighbor = pimNbt->getNeighborsByID(id)->getAddr();

	// Record in PIM Neighbor Table was found, can be deleted.
	if (pimNbt->deleteNeighbor(id))
		EV << "PIM::processNLTimer: Neighbor " << neighbor << "was removed from PIM neighbor table." << endl;

	delete nlt;
}

/**
 * PROCESS PIM PACKET
 *
 * The method processes new coming PIM packet. It has to find out
 * where packet has to be sent to. It looks to interface from which
 * packet is coming. According to interface ID, method findes record
 * in PIM Interface Table and gets info about PIM mode. According to mode
 * it send to appropriate PIM model.
 *
 * @param pkt New coming PIM packet.
 * @see PimInterface
 */
void PIMSplitter::processPIMPkt(PIMPacket *pkt)
{
	EV << "PIM::processPIMPkt" << endl;

	IPv4ControlInfo *ctrl = dynamic_cast<IPv4ControlInfo *>(pkt->getControlInfo());
	int intID = ctrl->getInterfaceId();
	int mode = 0;

	// find information about interface where packet came from
	PimInterface *pimInt = pimIft->getInterfaceByIntID(intID);
	if (pimInt != NULL)
		mode = pimInt->getMode();

	// according to interface PIM mode send packet to appropriate PIM module
	switch(mode)
	{
		case Dense:
			send(pkt, "pimDMOut");
			break;
		case Sparse:
			send(pkt, "pimSMOut");
			break;
		default:
			EV << "PIM::processPIMPkt: PIM is not enabled on interface number: "<< intID << endl;
			delete pkt;
			break;
	}
}

/**
 * HANDLE MESSAGE
 *
 * The method handles new coming message and process it according to
 * its type. Self message is timer. Other messages should be PIM packets.
 *
 * @param msg Pointer to message which has came to the module.
 * @see PIMTimer
 * @see sendHelloPkt()
 * @see processNLTimer()
 * @see PIMPacket
 * @see processHelloPkt()
 * @see processPIMPkt()
 */
void PIMSplitter::handleMessage(cMessage *msg)
{
	EV << "PimSplitter::handleMessage" << endl;

	// self message (timer)
   if (msg->isSelfMessage())
   {
	   PIMTimer *timer = check_and_cast <PIMTimer *> (msg);
	   if (timer->getTimerKind() == HelloTimer)
	   {
		   sendHelloPkt();
		   delete timer;
	   }
	   else if (timer->getTimerKind() == NeighborLivenessTimer)
	   {
		   processNLTimer(timer);
	   }
   }

   // PIM packet from network layer
   else if (dynamic_cast<PIMPacket *>(msg) && (strcmp(msg->getArrivalGate()->getName(), "transportIn") == 0))
   {
	   PIMPacket *pkt = check_and_cast<PIMPacket *>(msg);
	   IPv4ControlInfo *ctrl = dynamic_cast<IPv4ControlInfo *>(pkt->getControlInfo());
	   std::cout << simTime() << " " << hostname << "(" << ctrl->getInterfaceId() << "): " << pkt->getName() << " (" << ctrl->getSrcAddr() << ", " << ctrl->getDestAddr() << ")" << endl;
	   if (pkt->getType() == Hello)
		   processHelloPkt(pkt);
	   else
		   processPIMPkt(pkt);
   }

   // PIM packet from PIM mode, send to network layer
   else if (dynamic_cast<PIMPacket *>(msg) || dynamic_cast<MultData *>(msg))
   {
	   send(msg, "transportOut");
   }
   else
	   EV << "PIM:ERROR - bad type of message" << endl;
}

/**
 * INITIALIZE
 *
 * The method initialize ale structures (tables) which will use. It
 * subscribes to appropriate events of Notification Board. If there is
 * no PIM interface, PIM stops working. Otherwise it schedule Hello Timer.
 *
 * @param stage Stage of initialization.
 * @see MulticastRoutingTable
 * @see PIMTimer
 */
void PIMSplitter::initialize(int stage)
{
	// in stage 2 interfaces are registered
	// in stage 3 table pimInterfaces is built
	if (stage == 4)
	{
		hostname = par("hostname");

		// Pointer to routing tables, interface tables, notification board
		ift = InterfaceTableAccess().get();
		rt = AnsaRoutingTableAccess().get();
		nb = NotificationBoardAccess().get();
		pimIft = PimInterfaceTableAccess().get();
		pimNbt = PimNeighborTableAccess().get();

		// subscribtion of notifications (future use)
		nb->subscribe(this, NF_IPv4_NEW_MULTICAST);
		nb->subscribe(this, NF_INTERFACE_IPv4CONFIG_CHANGED);

		// is PIM enabled?
		if (pimIft->getNumInterface() == 0)
			return;
		else
			EV << "PIM is enabled on device " << hostname << endl;

		// to receive PIM messages, join to ALL_PIM_ROUTERS multicast group
		for (int i = 0; i < pimIft->getNumInterface(); i++)
		{
		    PimInterface *pimInterface = pimIft->getInterface(i);
		    pimInterface->getInterfacePtr()->ipv4Data()->joinMulticastGroup(IPv4Address("224.0.0.13")); // TODO use constant
		}

		// send Hello packets to PIM neighbors (224.0.0.13)
		PIMTimer *timer = new PIMTimer("Hello");
		timer->setTimerKind(HelloTimer);
		scheduleAt(simTime() + uniform(0,5), timer);
	}
}


/**
 * RECEIVE CHANGE NOTIFICATION
 *
 * The method from class Notification Board is used to catch its events.
 *
 * @param category Category of notification.
 * @param details Additional information for notification.
 * @see newMulticast()
 * @see igmpChange()
 */
void PIMSplitter::receiveChangeNotification(int category, const cPolymorphic *details)
{
	// ignore notifications during initialize
	if (simulation.getContextType()==CTX_INITIALIZE)
		return;

	// PIM needs details
	if (details == NULL)
		return;

	Enter_Method_Silent();
	printNotificationBanner(category, details);

	// according to category of event...
	switch (category)
	{
		// new multicast data appears in router
		case NF_IPv4_NEW_MULTICAST:
			EV <<  "PimSplitter::receiveChangeNotification - NEW MULTICAST" << endl;
			IPv4Datagram *datagram;
			datagram = check_and_cast<IPv4Datagram*>(details);
			newMulticast(datagram->getDestAddress(), datagram->getSrcAddress());
			break;

		// configuration of interface changed, it means some change from IGMP
		case NF_INTERFACE_IPv4CONFIG_CHANGED:
			EV << "PimSplitter::receiveChangeNotification - IGMP change" << endl;
			InterfaceEntry * interface = (InterfaceEntry *)(details);
			igmpChange(interface);
			break;
	}
}

/**
 * IGMP CHANGE
 *
 * The method is used to process notification about IGMP change. Splitter will find out which IP address
 * were added or removed from interface and will send them to appropriate PIM mode.
 *
 * @param interface Pointer to interface where IP address changed.
 * @see addRemoveAddr
 */
void PIMSplitter::igmpChange(InterfaceEntry *interface)
{
	int intId = interface->getInterfaceId();
	PimInterface * pimInt = pimIft->getInterfaceByIntID(intId);

	// save old and new set of multicast IP address assigned to interface
	if(pimInt)
	{
	vector<IPv4Address> multicastAddrsOld = pimInt->getIntMulticastAddresses();
	vector<IPv4Address> multicastAddrsNew = pimInt->deleteLocalIPs(interface->ipv4Data()->getReportedMulticastGroups());

	// vectors of new and removed multicast addresses
	vector<IPv4Address> add;
	vector<IPv4Address> remove;

	// which address was removed from interface
	for (unsigned int i = 0; i < multicastAddrsOld.size(); i++)
	{
		unsigned int j;
		for (j = 0; j < multicastAddrsNew.size(); j++)
		{
			if (multicastAddrsOld[i] == multicastAddrsNew[j])
				break;
		}
		if (j == multicastAddrsNew.size())
		{
			EV << "Multicast address " << multicastAddrsOld[i] << " was removed from the interface " << intId << endl;
			remove.push_back(multicastAddrsOld[i]);
		}
	}

	// which address was added to interface
	for (unsigned int i = 0; i < multicastAddrsNew.size(); i++)
	{
		unsigned int j;
		for (j = 0; j < multicastAddrsOld.size(); j++)
		{
			if (multicastAddrsNew[i] == multicastAddrsOld[j])
				break;
		}
		if (j == multicastAddrsOld.size())
		{
			EV << "Multicast address " << multicastAddrsNew[i] << " was added to the interface " << intId <<endl;
			add.push_back(multicastAddrsNew[i]);
		}
	}

	// notification about removed multicast address to PIM modules
	addRemoveAddr *addr = new addRemoveAddr();
	if (remove.size() > 0)
	{
		// remove new address
		for(unsigned int i = 0; i < remove.size(); i++)
			pimInt->removeIntMulticastAddress(remove[i]);

		// send notification
		addr->setAddr(remove);
		addr->setInt(pimInt);
        if (pimInt->getMode() == Dense)
            nb->fireChangeNotification(NF_IPv4_NEW_IGMP_REMOVED, addr);
        if (pimInt->getMode() == Sparse)
            nb->fireChangeNotification(NF_IPv4_NEW_IGMP_REMOVED_PIMSM, addr);
	}

	// notification about new multicast address to PIM modules
	if (add.size() > 0)
	{
		// add new address
		for(unsigned int i = 0; i < add.size(); i++)
			pimInt->addIntMulticastAddress(add[i]);

		// send notification
		addr->setAddr(add);
		addr->setInt(pimInt);
		if (pimInt->getMode() == Dense)
		    nb->fireChangeNotification(NF_IPv4_NEW_IGMP_ADDED, addr);
		if (pimInt->getMode() == Sparse)
		    nb->fireChangeNotification(NF_IPv4_NEW_IGMP_ADDED_PISM, addr);
	}
	}
}

/**
 * NEW MULTICAST
 *
 * The method process notification about new coming multicast data. According to
 * IP addresses it find all necessary info to create new entry for multicast
 * table.
 *
 * @param destAddr Destination IP address = multicast group IP address.
 * @param srcAddr Source IP address.
 * @see MulticastIPRoute
 */
void PIMSplitter::newMulticast(IPv4Address destAddr, IPv4Address srcAddr)
{
	EV << "PimSplitter::newMulticast - group: " << destAddr << ", source: " << srcAddr << endl;

	// find RPF interface for new multicast stream
	InterfaceEntry *inInt = rt->getInterfaceForDestAddr(srcAddr);
	if (inInt == NULL)
	{
		EV << "ERROR: PimSplitter::newMulticast(): cannot find RPF interface, routing information is missing.";
		return;
	}
	int rpfId = inInt->getInterfaceId();
	PimInterface *pimInt = pimIft->getInterfaceByIntID(rpfId);

	// if it is interface configured with PIM, create new route
	if (pimInt != NULL)
	{
		// create new multicast route
	    AnsaIPv4MulticastRoute *newRoute = new AnsaIPv4MulticastRoute();
		newRoute->setMulticastGroup(destAddr);
		newRoute->setOrigin(srcAddr);
		newRoute->setOriginNetmask(IPv4Address::ALLONES_ADDRESS);

		if (pimInt->getMode() == Dense)
		{
            // Directly connected routes to source does not have next hop
            // RPF neighbor is source of packet
            IPv4Address rpf;
            const IPv4Route *routeToSrc = rt->findBestMatchingRoute(srcAddr);
            if (routeToSrc->getSource() == IPv4Route::IFACENETMASK)
            {
                newRoute->addFlag(AnsaIPv4MulticastRoute::A);
                rpf = srcAddr;
            }
            // Not directly connected, next hop address is saved in routing table
            else
                rpf = rt->getGatewayForDestAddr(srcAddr);

            newRoute->setInInt(inInt, inInt->getInterfaceId(), rpf);
		}
		if (pimInt->getMode() == Sparse)
		    newRoute->setInInt(inInt, inInt->getInterfaceId(), IPv4Address("0.0.0.0"));

		// notification for PIM module about new multicast route
		if (pimInt->getMode() == Dense)
			nb->fireChangeNotification(NF_IPv4_NEW_MULTICAST_DENSE, newRoute);
		if (pimInt->getMode() == Sparse)
		    nb->fireChangeNotification(NF_IPv4_NEW_MULTICAST_SPARSE, newRoute);
	}
}
