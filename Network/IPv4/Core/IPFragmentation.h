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

/* -------------------------------------------------
	file: IPFragmentation.h
	Purpose: Header file for IPFragmentation
	------
	Responsibilities: 
	receive valid IP datagram from Routing or Multicast
	Fragment datagram if size > MTU [output port]
	send fragments to IPOutput[output port]
	author: Jochen Reber
	------------------------------------------------- */

#ifndef __IPFRAGMENTATION_H__
#define __IPFRAGMENTATION_H__

#include "RoutingTableAccess.h"
#include "IPDatagram.h"
#include "RoutingTable.h"
#include "ICMP.h"

class IPFragmentation: public RoutingTableAccess
{
private:
	int numOfPorts;
	simtime_t delay;

	void sendErrorMessage 
			(IPDatagram *, ICMPType type, ICMPCode code);
	void sendDatagramToOutput(IPDatagram *datagram);

public:
    Module_Class_Members(IPFragmentation, RoutingTableAccess, 
				ACTIVITY_STACK_SIZE);

	virtual void initialize();
	virtual void activity();
};

#endif

