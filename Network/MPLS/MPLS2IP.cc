/*******************************************************************
*
*	This library is free software, you can redistribute it
*	and/or modify
*	it under  the terms of the GNU Lesser General Public License
*	as published by the Free Software Foundation;
*	either version 2 of the License, or any later version.
*	The library is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*	See the GNU Lesser General Public License for more details.
*
*
*********************************************************************/
/*
*	File Name MPLS2IP.cc
*	LDP library
*	This file implements MPLS2IP class, the adapter between MPLS and IP layers
**/

#include <omnetpp.h>
#include <string.h>

#include "hook_types.h"
//#include "MPLSPacket.h"
#include "MPLS2IP.h"
#include "IPDatagram.h"
#include "LIBTableAccess.h"
#include "ConstType.h"
#include "hook_types.h"
#include "IPDatagram.h"



Define_Module_Like( MPLS2IP, NetworkInterface );

void MPLS2IP::initialize()
{
	// ProcessorAccess::initialize();

	delay = par("procdelay");
}

void MPLS2IP::activity()
{
	cMessage *msg;

	while(true)
	{
		msg = receive();

		if (!strcmp(msg->arrivalGate()->name(), "ipOutputQueueIn"))
		{
			cMessage *nwiIdleMsg = new cMessage();

			nwiIdleMsg->setKind(NWI_IDLE);

                        //wait(delay);
                        send(msg, "physicalOut");

			send(nwiIdleMsg, "ipOutputQueueOut");

		}
		else
		{
			//wait(delay);
			send(msg, "ipInputQueueOut");

		}

}
}
