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
	file: RoutingCore.h
	Purpose: Header file for RoutingModule

	Responsibilities: 
	Receive correct IP datagram
	if source routing option is on, use next source addr. as dest. addr.
	map IP address on output port, use static routing table
	if destination address is not in routing table, 
	throw datagram away and notify ICMP
	process record route and timestamp options, if applicable
	send to local Deliver if dest. addr. = 127.0.0.1 
	or dest. addr. = NetworkCardAddr.[]
	send datagram with Multicast addr. to Multicast module
	otherwise, send to Fragmentation module

	comments: 
	IP options should be handled here, but are currently not
	implemented (20.5.00)
	IPForward not implemented

	author: Jochen Reber
	date: 20.5.00
*/

#ifndef __RoutingCore_H__
#define __RoutingCore_H__

#include "RoutingTableAccess.h"
#include "RoutingTable.h"
#include "ICMP.h"

class RoutingCore: public RoutingTableAccess 
{
private:
	bool IPForward;
	simtime_t delay;
	bool hasHook;

	void sendErrorMessage (IPDatagram *, ICMPType, ICMPCode);
public:
	Module_Class_Members(RoutingCore, RoutingTableAccess, 
			ACTIVITY_STACK_SIZE);

	virtual void initialize();
	virtual void activity();
};

#endif
