// $Header$
//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

/*
    file: ICMP.cc
    Purpose: Implementation of the ICMP Module
    Responsibilities:
    receive ICMP message from local deliver

    process ICMP message:
    echo/timestamp reply - strip ICMP header and send to pingOut
    echo/timestamp request - create reply and send to IPSend
    destination unreachable - send to errorOut
    time exceeded - send to errorOut
    parameter problem - send to errorOut
    redirect - ignored

    error messages ignored if destination multicast address
    receive error message from Routing:
    create destination unreachable message and send to IPSend
    receive error message from PreRouting:
    create parameter problem or time exceeded message and
    send to IPSend as IPInterfacePacket
    receive echo/timestamp message from pingIn:
    encapsulate in ICMP header and send to IPSend as IPInterfacePacket

	length() rules(in bit): ping messages have 8*20 bits length;
			  error messages have 8*4 bits ICMP header,
			  full IP header of error datagram (+options)
			  + 8*8 bits payload of the error datagram
			  (usually transport layer header)

    author: Jochen Reber
*/

#include <omnetpp.h>
#include <string.h>

#include "ICMP.h"
//#include "ProcessorManager.h"

Define_Module( ICMP );

/*  ----------------------------------------------------------
		overloaded functions of classes
		derived from cMessage
    ----------------------------------------------------------  */

// ICMPQueryType
ICMPQueryType::ICMPQueryType(const ICMPQueryType& qt) : cObject()
{
	setName(qt.name());
	operator=( qt );
}

ICMPQueryType& ICMPQueryType::operator=( const ICMPQueryType& qt )
{
	cObject::operator=(qt);
	timestampValid = qt.timestampValid;
	identifier = qt.identifier;
	seq = qt.seq;
	originateTimestamp = qt.originateTimestamp;
    receiveTimestamp = qt.receiveTimestamp;
    transmitTimestamp = qt.transmitTimestamp;

	return *this;

}

// ICMPMessage
ICMPMessage::ICMPMessage()
{
	type = (ICMPType)0;
	code = 0;
	isErrorMessage = false;
	query = NULL;
}

ICMPMessage::ICMPMessage(const ICMPMessage& msg)
{
	setName(msg.name());
	operator= (msg);
}

ICMPMessage::~ICMPMessage()
{
	if (isErrorMessage)
	{
		if (errorDatagram)
			delete(errorDatagram);
	} else {
		if (query)
			delete(query);
	}
}

ICMPMessage& ICMPMessage::operator=( const ICMPMessage& msg)
{
	cPacket::operator=(msg);
	type = msg.type;
	code = msg.code;
	isErrorMessage = msg.isErrorMessage;
	if (isErrorMessage)
	{
		if (msg.errorDatagram)
			errorDatagram = (IPDatagram *)msg.errorDatagram->dup();
	} else {
		if (msg.query)
			query = (ICMPQueryType *)msg.query->dup();
	}

	return *this;
}


/*  ----------------------------------------------------------
        Public Functions
    ----------------------------------------------------------  */

void ICMP::initialize()
{
	RoutingTableAccess::initialize();
	delay = par("procdelay");

}

void ICMP::activity()
{
	cMessage *msg;
	cGate *arrivalGate;

	while(true)
	{
		msg = receive();

		arrivalGate = msg->arrivalGate();

		wait(delay);
		/* error message from Routing, PreRouting, Fragmentation
			or IPOutput: send ICMP message */
		if (!strcmp(arrivalGate->name(), "preRoutingIn")
				|| !strcmp(arrivalGate->name(), "routingIn")
				|| !strcmp(arrivalGate->name(), "fragmentationIn")
				|| !strcmp(arrivalGate->name(), "ipOutputIn"))
		{
			processError(msg);
			// releaseKernel();
			continue;
		}

		// process arriving ICMP message
		if (!strcmp(arrivalGate->name(), "localIn"))
		{
			processICMPMessage((IPInterfacePacket *)msg);
			// releaseKernel();
			continue;
		}

		// request from application
		if (!strcmp(arrivalGate->name(), "pingIn"))
		{
			// claimKernel();
			sendEchoRequest(msg);
			// releaseKernel();
			continue;
		}
	} // end while
}


/*  ----------------------------------------------------------
        Private Functions
    ----------------------------------------------------------  */
void ICMP::processError(cMessage *msg)
{
	int type = msg->par("ICMPType");
	int code = msg->par("ICMPCode");
	IPDatagram *origDatagram =
				(IPDatagram *) msg->parList().get("datagram");

	// don't send ICMP error messages for multicast messages
	if (rt->isMulticastAddr(origDatagram->destAddress()))
	{
		delete msg;
		return;
	}

	ICMPMessage *errorMessage = new ICMPMessage();
	IPDatagram *e = (IPDatagram *)origDatagram->dup();

	errorMessage->type = (ICMPType) type;
	errorMessage->code = code;
	errorMessage->isErrorMessage = true;
	errorMessage->errorDatagram = e;
	// ICMP message length: see above
	errorMessage->setLength(8 * (4 + e->headerLength() + 8));

	// origDatagram should get deleted here, e not
	delete (msg);

	// do not reply with error message to error message
	if (e->protocol() == IP_PROT_ICMP)
	{
		ICMPMessage *recICMPMsg =
				(ICMPMessage *)e->encapsulatedMsg();
		if (recICMPMsg->isErrorMessage)
		{
			// deletes errorMessage, e and recICMPMsg
			delete( errorMessage );
			return;
		}
	}

	sendInterfacePacket(errorMessage, e->srcAddress());

	// debugging information
	ev << "*** ICMP: send error message: "
		<< (int)errorMessage->type << " / "
		<< errorMessage->code
		<< "\n";
	//breakpoint("ICMP: send error message");
}

/*
	error Messages are simply forwarded to errorOut
*/
void ICMP::processICMPMessage(IPInterfacePacket *interfacePacket)
{
	ICMPMessage	*icmpmsg =
			(ICMPMessage *) interfacePacket->decapsulate();
	// use source of ICMP message as destination for reply
	IPAddrChar src;

	strcpy(src, interfacePacket->srcAddr());
	delete( interfacePacket );

	switch (icmpmsg->type)
	{
		case ICMP_ECHO_REPLY:
			recEchoReply(icmpmsg);
			break;
		case ICMP_DESTINATION_UNREACHABLE:
			errorOut(icmpmsg);
			break;
    	case ICMP_REDIRECT:
			errorOut(icmpmsg);
			break;
		case ICMP_ECHO_REQUEST:
			recEchoRequest(icmpmsg, src);
			break;
    	case ICMP_TIME_EXCEEDED:
			errorOut(icmpmsg);
			break;
    	case ICMP_PARAMETER_PROBLEM:
			errorOut(icmpmsg);
			break;
    	case ICMP_TIMESTAMP_REQUEST:
			recEchoRequest(icmpmsg, src);
			break;
    	case ICMP_TIMESTAMP_REPLY:
			recEchoReply(icmpmsg);
			break;
		default:
			ev << "*** ICMP: no type found! "
				<< (int)icmpmsg->type << "\n";
			delete(icmpmsg);
	}
}

void ICMP::errorOut(ICMPMessage *icmpmsg)
{
	send(icmpmsg, "errorOut");
}


/*  ----------------------------------------------------------
       Echo/Timestamp request and reply ICMP messages
    ----------------------------------------------------------  */
void ICMP::recEchoRequest
	(ICMPMessage *request, const char *dest)
{
	ICMPMessage *reply = new ICMPMessage(*request);
	bool timestampValid = request->query->timestampValid;

	reply->type = timestampValid ?
				  ICMP_TIMESTAMP_REPLY : ICMP_ECHO_REPLY;
	reply->code = 0;
	reply->query = new ICMPQueryType(*(request->query));
	reply->setLength(8*20);

	delete(request);

	if (timestampValid)
	{
		reply->query->receiveTimestamp = simTime();
		reply->query->transmitTimestamp = simTime();
	}

	sendInterfacePacket(reply, dest);
}

void ICMP::recEchoReply (ICMPMessage *reply)
{
	cMessage *msg = new cMessage;
	ICMPQueryType *echo_info = new ICMPQueryType (*(reply->query));

	delete(reply);

	echo_info->setName("echo_info");
	msg->parList().add( echo_info );
	send(msg, "pingOut");
}

void ICMP::sendEchoRequest(cMessage *msg)
{
	ICMPMessage *icmpmsg = new ICMPMessage();
	ICMPQueryType *echo_info =
			(ICMPQueryType *)msg->parList().get("echo_info");
	IPAddrChar dest;

	icmpmsg->type = echo_info->timestampValid ?
			ICMP_TIMESTAMP_REQUEST : ICMP_ECHO_REQUEST;
	icmpmsg->code = 0;
	icmpmsg->isErrorMessage = false;
	icmpmsg->query = new ICMPQueryType(*echo_info);
	icmpmsg->setLength(8*20);

	strcpy(dest, msg->par("destination_address"));
	delete(msg);

	sendInterfacePacket(icmpmsg, dest);
}

void ICMP::sendInterfacePacket(ICMPMessage *msg, const char *dest)
{
	IPInterfacePacket *interfacePacket = new IPInterfacePacket;

	interfacePacket->encapsulate(msg);
	interfacePacket->setDestAddr(dest);
	interfacePacket->setProtocol(IP_PROT_ICMP);

	send(interfacePacket, "sendOut");
}

