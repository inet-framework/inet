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
	file: IPMulticast.h
	Purpose: Header file for IP Multicast

	Responsibilities:
	receive datagram with Multicast address from Routing
	duplicate datagram if it is sent to more than one output port
	map multicast address on output port, use multicast routing table
	send copy to  local deliver, if
	NetworkCardAddr.[] is part of multicast address
	if entry in multicast routing table requires tunneling,
	send to Tunneling module
	otherwise send to Fragmentation module

	receive IGMP message from LocalDeliver
	update multicast routing table

	author: Jochen Reber
*/

#ifndef __IPMULTICAST_H__
#define __IPMULTICAST_H__

#include "RoutingTableAccess.h"
#include "IPDatagram.h"

class IPMulticast: public RoutingTableAccess
{
private:
	bool IPForward;
	simtime_t delay;

public:
	Module_Class_Members(IPMulticast, RoutingTableAccess, 
				ACTIVITY_STACK_SIZE);

	virtual void initialize();
	virtual void activity();
};

#endif

