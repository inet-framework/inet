// -*- C++ -*-
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
	file: ICMP.h
	Purpose: Header file for the ICMP-Module
	Responsibilities:
	receive ICMP message from local deliver

	process ICMP message:
	echo/timestamp reply - strip ICMP header and send to pingOut
	echo/timestamp request - create reply and send to IPSend
	destination unreachable - send to errorOut
	time exceeded - send to errorOut
	parameter problem - send to errorOut
	redirect - ignored

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

/*
	Interface formats:

	errorMessage from other modules
	(Fragmentation, PreRouting, Routing):
	type: cMessage
	parameters: "ICMPType" -- ICMPType
				"ICMPCode" -- ICMPCode (int)
	parList:	"datagram" -- IPDatagram

	message format for pingIn/pingOut:
	type: cMessage
	parameters: "destination_address" -- char[] 
	parList:	"echo_info" -- ICMPQueryType
					identifiery, sequence number and timestampValid
					need to be set by app

	message format to errorOut:
	type: ICMPMessage
*/

#ifndef __ICMP_H__
#define __ICMP_H__

#include "RoutingTableAccess.h"
#include "IPDatagram.h"

/* 	-----------------------------------------------------------
		Enumerations and Types
 	----------------------------------------------------------- 	*/

enum ICMPType 
{
	ICMP_ECHO_REPLY = 0,
	ICMP_DESTINATION_UNREACHABLE = 3,
	ICMP_REDIRECT = 5,
	ICMP_ECHO_REQUEST = 8,
	ICMP_TIME_EXCEEDED = 11,
	ICMP_PARAMETER_PROBLEM = 12,
	ICMP_TIMESTAMP_REQUEST = 13,
	ICMP_TIMESTAMP_REPLY = 14
};

typedef int ICMPCode;

/*  -------------------------------------------------
        structures
    -------------------------------------------------   */

class ICMPQueryType: public cObject
{
public:
	ICMPQueryType(const ICMPQueryType& );
	virtual ICMPQueryType& operator=( const ICMPQueryType& );
	virtual cObject *dup() const
		{ return new ICMPQueryType(*this); }

	bool timestampValid;
	int identifier;
	int seq;
	// timestamp fields only used in timestamp req/reply
	simtime_t originateTimestamp;
	simtime_t receiveTimestamp;
	simtime_t transmitTimestamp;
};

/* 	-----------------------------------------------------------
		ICMP message class
 	----------------------------------------------------------- 	*/
class ICMPMessage: public cPacket
{
public:
	ICMPMessage();
	ICMPMessage(const ICMPMessage &);
	~ICMPMessage();
	virtual ICMPMessage& operator=( const ICMPMessage & );
	virtual cObject *dup() const
		{ return new ICMPMessage(*this); }

	ICMPType type;
	int code;
	bool isErrorMessage;
	union {
		ICMPQueryType *query;
		IPDatagram *errorDatagram;
	};
};

/* 	-----------------------------------------------------------
		ICMP Module	
 	----------------------------------------------------------- 	*/
class ICMP: public RoutingTableAccess
{
private:
	simtime_t delay;

	void processError (cMessage *);
	void processICMPMessage(IPInterfacePacket *);
	void errorOut(ICMPMessage *);
	void recEchoRequest (ICMPMessage *, const char *dest);
	void recEchoReply (ICMPMessage *);
	void sendEchoRequest(cMessage *);
	void sendInterfacePacket(ICMPMessage *, const char *dest);

public:
	Module_Class_Members(ICMP, RoutingTableAccess, ACTIVITY_STACK_SIZE);

	virtual void initialize();
	virtual void activity();
};

#endif

