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
    file: Dequeue.cc
    Purpose: Implementation for Dequeue Modul of the Output Queue
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

#include <omnetpp.h>

#include "hook_types.h"
#include "Dequeue.h"

Define_Module( Dequeue );

void Dequeue::initialize()
{
	// ProcessorAccess::initialize();

	delay = par("procdelay");
	nwi_idle = true;
	queue_empty = true;
}

void Dequeue::activity()
{

	cMessage *msg;

	while(true)
	{
		msg = receive();

		if (!strcmp(msg->arrivalGate()->name(), "fromNW"))
		{
			nwi_idle = true;
			delete msg;
			sendRequest();
			continue;
		}


		if (!strcmp(msg->arrivalGate()->name(), "inDirekt"))
		{
			queue_empty = false;
			delete msg;
			sendRequest();
			continue;
		}

	}
}

/* private function: send Request_Packet-Message if
		Network Interface is idle and Queue is not empty */
void Dequeue::sendRequest()
{
	if (nwi_idle && !queue_empty)
	{
		cMessage *requestMsg = new cMessage("Request Message");
		requestMsg->setKind(REQUEST_PACKET);

		// claimKernel();
		wait(delay);
		send(requestMsg, "toDeqHook");
		getReplyFromDeqHook();
	}

}

void Dequeue::getReplyFromDeqHook()
{
	cMessage *msg;
	msg = receive();
	ASSERT(dfmsg->arrivedOn("fromDeqHook"));  // FIXME revise this

	switch(msg->kind())
	{
		case MK_PACKET:
			send(msg, "toNW");
			nwi_idle = false;
			break;
		case NO_PACKET:
			delete msg;
			queue_empty = true;
			break;
		default:
			ev << "Error: invalid Message kind at Dequeue: "
				<< msg->kind();
	}
	// releaseKernel();

}

