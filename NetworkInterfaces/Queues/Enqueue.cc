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
    file: Enqueue.cc
    Purpose: Implementation for Enqueue Modul of the Output Queue
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


#include "hook_types.h"
#include "Enqueue.h"

Define_Module( Enqueue );


void Enqueue::initialize()
{
    // ProcessorAccess::initialize();
    delay = par("procdelay");
}


void Enqueue::activity()
{

	cMessage *msg;

	while(true)
	{
		msg = receive();

		// claimKernel();
		send(msg, "toEnqHook");

		wait(delay);

		msg = receive();
		ASSERT(dfmsg->arrivedOn("fromEnqHook"));  // FIXME revise this
		switch(msg->kind())
		{
			case PACKET_ENQUEUED:
				sendWakeupCall();
				break;
			case DISCARD_PACKET:
				break;
			default:
				ev << "Error: invalid Message kind at Enqueue: "
					<< msg->kind();
		} // end switch
		delete msg;
		// releaseKernel();

	} // end while
}

/*
	private functions
*/

/* function sends a wakeup call to the Dequeue Module
	with direkt send */
void Enqueue::sendWakeupCall()
{
	cMessage *wakeupMsg = new cMessage();
	cSimpleModule *dequeueModule =
		(cSimpleModule *)parentModule()->findObject("deq");

	wakeupMsg->setKind(WAKEUP_PACKET);
	sendDirect(wakeupMsg, 0, dequeueModule, "inDirekt");
}

