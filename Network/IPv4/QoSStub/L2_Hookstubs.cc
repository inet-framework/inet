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
	file: Hookstubs.cc
        Purpose: Empty test implementation for L2 queue hooks
        Responsibilities: define L2_DequeueHook and L2_EnqueueHook
        do nothing, just exist packet sink
        please replace with QoS modules later

	author: Jochen Reber
*/
	
#include <omnetpp.h>

#include "hook_types.h"
#include "L2_Hookstubs.h"

static cQueue outputQueue("Output Queue");

Define_Module( L2_EnqueueHook );

void L2_EnqueueHook::handleMessage(cMessage *msg)
{
	cMessage *enqueueMsg = new cMessage();
	enqueueMsg->setKind(PACKET_ENQUEUED);

	outputQueue.insert(msg);
	send(enqueueMsg, "toEnqueue");

}

Define_Module( L2_DequeueHook );

void L2_DequeueHook::handleMessage(cMessage *msg)
{
	delete msg;

	if (!outputQueue.empty())
	{
		cMessage *packet = (cMessage *)outputQueue.pop();
		send(packet, "toDequeue");
		
	} else 
	{
		cMessage *noPacketMsg = new cMessage("noPacket");
		noPacketMsg->setKind(NO_PACKET);	
		send(noPacketMsg, "toDequeue");
	}
}

