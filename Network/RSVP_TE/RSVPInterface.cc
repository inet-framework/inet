/*******************************************************************
*
*	This library is free software, you can redistribute it
*	and/or modify
*	it under  the terms of the GNU Lesser General Public License
*	as published by the Free Software Foundation;
*	either version 2 of the License, or any later version.
*	The library is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*	See the GNU Lesser General Public License for more details.
*
*
*********************************************************************/


/*
*	File Name RSVPInterface.cc
*	RSVP-TE library
*	This file implements RSVPInterface class
**/

#include <omnetpp.h>
#include <string.h>

#include "rsvp_message.h"
#include "RSVPInterface.h"
//#include "ProcessorManager.h"
#include "IPInterfacePacket.h"

Define_Module( RSVPInterface );

void RSVPInterface::initialize()
{

}

void RSVPInterface::activity()
{

	cMessage *msg; // the message that will be received

	while(true)
	{

		msg = receive();

		// notify ProcessorManager to begin task
		//claimKernel();

		// received from IP layer
		if (strcmp(msg->arrivalGate()->name(), "from_ip") == 0)
		{
			processMsgFromIp(msg);

		}  else // received from application layer
		{
			processMsgFromApp(msg);
		}

		// notify ProcessorManager to end task
		//releaseKernel();

	}
}



void RSVPInterface::processMsgFromIp(cMessage *msg)
{
	//int i;
	//int applicationNo = -1;
	//int port;
	PathMessage *pMsg;
	ResvMessage *rMsg;
	PathTearMessage *ptMsg;
	PathErrorMessage *peMsg;
	//ResvTearMessage *rtMsg;
	//ResvErrorMessage *reMsg;

	IPInterfacePacket *iPacket = (IPInterfacePacket *)msg;

	TransportPacket* tpacket = (TransportPacket*)(iPacket->decapsulate());
	cMessage* rsvpMsg =(cMessage*)( tpacket->par("rsvp_data").objectValue());
	int msgKind = rsvpMsg->kind();

	int peerInf =0;

	switch(msgKind)
	{
	case PATH_MESSAGE:

		   pMsg = new PathMessage();
  		   pMsg->setContent((PathMessage*)rsvpMsg);

  		   if(rsvpMsg->hasPar("peerInf"))
  		   {
  		   	peerInf = rsvpMsg->par("peerInf").longValue();
  		   	pMsg->addPar("peerInf") = peerInf;
  		   }

  		   send(pMsg, "to_rsvp_app");

		break;

	case RESV_MESSAGE:
		rMsg = new ResvMessage();
  		rMsg->setContent((ResvMessage*)rsvpMsg);
  		send(rMsg, "to_rsvp_app");

		break;

	case PTEAR_MESSAGE:
		ptMsg = new PathTearMessage();
		ptMsg->setContent((PathTearMessage*)rsvpMsg);
		send(ptMsg, "to_rsvp_app");

		break;

	case PERROR_MESSAGE:
		peMsg = new PathErrorMessage();
		peMsg->setContent((PathErrorMessage*)rsvpMsg);
		send(peMsg, "to_rsvp_app");
		break;

	default:
		ev << "Error: Unrecognised RSVP TE message types \n";
		delete msg;
		break;

	}





}

void RSVPInterface::processMsgFromApp(cMessage *msg)
{
	IPAddrChar src_addr, dest_addr;
	TransportPacket* tpacket = new TransportPacket(*msg);
	tpacket->addPar("rsvp_data") = msg;

	IPInterfacePacket *iPacket = new IPInterfacePacket();

	iPacket->encapsulate(tpacket);

	strcpy(src_addr,
		msg->hasPar("src_addr")
		? msg->par("src_addr").stringValue()
		: MY_ERROR_IP_ADDRESS);
	strcpy(dest_addr,
		msg->hasPar("dest_addr")
		? msg->par("dest_addr").stringValue()
		: MY_ERROR_IP_ADDRESS);

	// encapsulate udpPacket into an IPInterfacePacket
	iPacket->setDestAddr(dest_addr);
	if (strcmp(MY_ERROR_IP_ADDRESS, src_addr))
	{
		iPacket->setSrcAddr(src_addr);
	}
	iPacket->setProtocol(IP_PROT_RSVP);

	send(iPacket,"to_ip");


}
