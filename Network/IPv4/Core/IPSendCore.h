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

/* 	-------------------------------------------------
    file: IPSendCore.h
    Purpose: Header file for IPSendCore
    ------
	Responsibilities: 
	receive IPInterfacePacket from Transport layer or ICMP 
	or Tunneling (IP tunneled datagram)
	take out control information
	encapsulate data packet in IP datagram
	set version
	set ds.codepoint 
	set TTL to constant value
	choose and set fragmentation identifier
	set fragment offset = 0, more fragments = 0 
	(fragmentation occurs in IPFragmentation)
	set 'don't Fragment'-bit to received value (default 0)
	set Protocol to received value
	set destination address to received value
	send datagram to Routing

	if IPInterfacePacket is invalid (e.g. invalid source address),
	 it is thrown away without feedback 

	author: Jochen Reber
	------------------------------------------------- */

#ifndef __IPSENDCORE_H__
#define __IPSENDCORE_H__

#include "RoutingTableAccess.h"
#include "IPInterfacePacket.h"
#include "RoutingTable.h"


class IPSendCore: public RoutingTableAccess 
{
private:
	int defaultTimeToLive;
	int defaultMCTimeToLive;
	/* type and value of curFragmentId is
		just incremented per datagram sent out;
		Transport layer cannot give values */
	long curFragmentId;
	simtime_t delay;
	bool hasHook;

	void sendDatagram(IPInterfacePacket *);
public:
    Module_Class_Members(IPSendCore, RoutingTableAccess, 
				ACTIVITY_STACK_SIZE);

	virtual void initialize();
    virtual void activity();
};

#endif

