//
// Copyright (C) 2013 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 3
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//
// Authors: Veronika Rybova, Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)

#include "IPv4Datagram.h"
#include "IPSocket.h"
#include "PIMSplitter.h"
#include "PIMRoute.h"
#include "IPv4RoutingTableAccess.h"

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
	for (int i = 0; i < pimIft->getNumInterfaces(); i++)
	{
		intID = pimIft->getInterface(i)->getInterfaceId();
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
    InterfaceEntry *ie = ift->getInterfaceById(ctrl->getInterfaceId());
    IPv4Address address = ctrl->getSrcAddr();

    PIMNeighbor *neighbor = pimNbt->findNeighbor(ctrl->getInterfaceId(), ctrl->getSrcAddr());

    // new neighbor (it is not in PIM neighbor table)
    // insert new neighbor to table
    // set Neighbor Livness Timer
    if (!neighbor)
    {
        neighbor = new PIMNeighbor(ie, address, msg->getVersion());
        pimNbt->addNeighbor(neighbor);
        EV << "PimSplitter::New Entry was added: addr = " << neighbor->getAddress() << ", iftID = " << neighbor->getInterfaceId() << ", ver = " << neighbor->getVersion() << endl;
    }
    // neighbor is already in PIM neighbor table
    // refresh Neighbor Livness Timer
    else
    {
        pimNbt->restartLivenessTimer(neighbor);
    }

    delete msg;
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
	PIMInterface *pimInt = pimIft->getInterfaceById(intID);
	if (pimInt != NULL)
		mode = pimInt->getMode();

	// according to interface PIM mode send packet to appropriate PIM module
	switch(mode)
	{
		case PIMInterface::DenseMode:
			send(pkt, "pimDMOut");
			break;
		case PIMInterface::SparseMode:
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
   }
   else {
       std::string arrivalGate = msg->getArrivalGate()->getName();

       // PIM packet from network layer
       if (dynamic_cast<PIMPacket *>(msg) && arrivalGate == "transportIn")
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
       else if (arrivalGate == "pimSMIn" || arrivalGate == "pimDMIn")
       {
           send(msg, "transportOut");
       }
       else
           EV << "PIM:ERROR - bad type of message" << endl;
   }

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
    cSimpleModule::initialize(stage);

	// in stage 2 interfaces are registered
	// in stage 3 table pimInterfaces is built
    if (stage == INITSTAGE_LOCAL)
    {
        // Pointer to routing tables, interface tables, notification board
        ift = InterfaceTableAccess().get();
        rt = IPv4RoutingTableAccess().get();
        pimIft = PIMInterfaceTableAccess().get();
        pimNbt = PIMNeighborTableAccess().get();

        // subscribtion of notifications (future use)
        cModule *host = findContainingNode(this);
        if (!host)
            throw cRuntimeError("PIMSplitter: containing node not found.");

        hostname = host->getName();
        host->subscribe(NF_IPv4_NEW_MULTICAST, this);
    }
    else if (stage == INITSTAGE_TRANSPORT_LAYER)
    {
        IPSocket ipSocket(gate("transportOut"));
        ipSocket.registerProtocol(IP_PROT_PIM);
    }
    else if (stage == INITSTAGE_ROUTING_PROTOCOLS)
	{
		// is PIM enabled?
		if (pimIft->getNumInterfaces() == 0)
			return;
		else
			EV << "PIM is enabled on device " << hostname << endl;


		// to receive PIM messages, join to ALL_PIM_ROUTERS multicast group
		for (int i = 0; i < pimIft->getNumInterfaces(); i++)
		{
		    PIMInterface *pimInterface = pimIft->getInterface(i);
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
void PIMSplitter::receiveSignal(cComponent *source, simsignal_t signalID, cObject *details)
{
	// ignore notifications during initialize
	if (simulation.getContextType()==CTX_INITIALIZE)
		return;

	// PIM needs details
	if (details == NULL)
		return;

	Enter_Method_Silent();
    printNotificationBanner(signalID, details);

    // new multicast data appears in router
    if (signalID == NF_IPv4_NEW_MULTICAST)
    {
        EV <<  "PimSplitter::receiveChangeNotification - NEW MULTICAST" << endl;
        IPv4Datagram *datagram;
        datagram = check_and_cast<IPv4Datagram*>(details);
        newMulticast(datagram->getDestAddress(), datagram->getSrcAddress());
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
	PIMInterface *pimInt = pimIft->getInterfaceById(rpfId);

	// if it is interface configured with PIM, create new route
	if (pimInt != NULL)
	{
		// create new multicast route
	    PIMMulticastRoute *newRoute = new PIMMulticastRoute();
		newRoute->setMulticastGroup(destAddr);
		newRoute->setOrigin(srcAddr);
		newRoute->setOriginNetmask(IPv4Address::ALLONES_ADDRESS);

		if (pimInt->getMode() == PIMInterface::DenseMode)
		{
            // Directly connected routes to source does not have next hop
            // RPF neighbor is source of packet
            IPv4Address rpf;
            const IPv4Route *routeToSrc = rt->findBestMatchingRoute(srcAddr);
            if (routeToSrc->getSourceType() == IPv4Route::IFACENETMASK)
            {
                newRoute->addFlag(PIMMulticastRoute::A);
                rpf = srcAddr;
            }
            // Not directly connected, next hop address is saved in routing table
            else
                rpf = rt->getGatewayForDestAddr(srcAddr);

            newRoute->setInInt(inInt, inInt->getInterfaceId(), rpf);
		}
		if (pimInt->getMode() == PIMInterface::SparseMode)
		    newRoute->setInInt(inInt, inInt->getInterfaceId(), IPv4Address("0.0.0.0"));

		// notification for PIM module about new multicast route
		if (pimInt->getMode() == PIMInterface::DenseMode)
			emit(NF_IPv4_NEW_MULTICAST_DENSE, newRoute);
		if (pimInt->getMode() == PIMInterface::SparseMode)
		    emit(NF_IPv4_NEW_MULTICAST_SPARSE, newRoute);
	}
}
