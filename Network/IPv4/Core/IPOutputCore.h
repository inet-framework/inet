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
	file: IPOutputCore.h
	Purpose: Header file for IPOutput core module
	------
	Responsibilities:
	receive complete datagram from IPFragmentation
        hop counter check
            -> throw away and notify ICMP if ttl==0
        otherwise  send it on to output queue
	author: Jochen Reber
	------------------------------------------------- 	*/

#ifndef __IPOUTPUTCORE_H__
#define __IPOUTPUTCORE_H__

#include "basic_consts.h"
//#include "ProcessorAccess.h"
#include "IPDatagram.h"
#include "ICMP.h"

class IPOutputCore : public cSimpleModule   // was ProcessorAccess
{
private:
	simtime_t delay;
	bool hasHook;

	void sendErrorMessage( IPDatagram *, ICMPType, ICMPCode);
public:
    Module_Class_Members(IPOutputCore, cSimpleModule, ACTIVITY_STACK_SIZE);

	virtual void initialize();
    virtual void activity();

};

#endif

