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
	file: Enqueue.h
	Purpose: Header file for Enqueue Modul of the Output Queue
	Responsibilities:
		receive IPDatagram from IP layer:
		claim Kernel
		send IPDatagram to EnqueueHook

		receive IPDatagram or Discard_Packet-Message from EnqueueHook:
		ignore Discard_Packet-Message
		on IPDatagram: notify Dequeue directly with 
				WakeUp_Queue-Message
		release Kernel

	author: Jochen Reber
*/

#ifndef __ENQUEUE_H__
#define __ENQUEUE_H__

#include <omnetpp.h>

#include "basic_consts.h"
#include "ProcessorAccess.h"

class Enqueue: public ProcessorAccess
{
private:
	simtime_t delay;

	void sendWakeupCall();
public:
	Module_Class_Members(Enqueue, ProcessorAccess, ACTIVITY_STACK_SIZE);

	virtual void initialize();
	virtual void activity();
};

#endif

