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
	file: Dequeue.h
	Purpose: Header file for Dequeue Modul of the Output Queue
	Responsibilities:
		receive Networkinterface Idle-Message from Network Interface:
			set nwi_idle = true

		receive Wakeup_Queue-Message from Direkt-In-Gate:
			set queue_empty = false

		==> if  nwi_idle && !queue_empty:
			claim Kernel
			send Request_Packet-Message to Dequeue-Hook

		receive IPDatagram from DequeueHook:
		send IPDatagram to Network Interface
		set nwi_idle = false
		release Kernel
		
		receive NoPacket-Message from DequeueHook:
		set queue_empty = true
		release Kernel

	author: Jochen Reber
*/

#ifndef __DEQUEUE_H__
#define __DEQUEUE_H__

#include <omnetpp.h>

#include "basic_consts.h"
#include "ProcessorAccess.h"

class Dequeue: public ProcessorAccess 
{
private:
	bool nwi_idle;
	bool queue_empty;
	simtime_t delay;

	void getReplyFromDeqHook();
	void sendRequest();
public:
	Module_Class_Members(Dequeue, ProcessorAccess, ACTIVITY_STACK_SIZE);

	virtual void initialize();
	virtual void activity();
};

#endif

