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
	file: IPTunneling.h
	Purpose: Header file for IP Tunneling
	------
	Responsibilities:
	receive message from IPMulticast
	set Tunnel destination address
	set Protocol field
	send new IPInterfacePacket to IPSend to be newly encapsulated

	author: Jochen Reber

	comment: interface from IPMulticast still needs doc
*/

#ifndef __IPTUNNELING_H__
#define __IPTUNNELING_H__

#include "IPDatagram.h"
//#include "ProcessorAccess.h"

class IPTunneling : public cSimpleModule   // was ProcessorAccess
{
private:
	simtime_t delay;

public:
	Module_Class_Members(IPTunneling, cSimpleModule, ACTIVITY_STACK_SIZE);

	virtual void initialize();
	virtual void activity();
};

#endif

