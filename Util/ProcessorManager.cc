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

/* 	------------------------------------------------
	file: ProcessorManager.cc
	Purpose: Contains both Implementation of Processor 
	Manager and Processor Messages
	relies on direct send in and out
	set numOfProcessors=1 for single processor node
	set numOfProcessors=0 to disable processor check
	author: Jochen Reber
	date: 16.6.00
	------------------------------------------------ */

#include <omnetpp.h>

#include "ProcessorManager.h"


/* 	------------------------------------------------
		Processor Manager: Public functions
	------------------------------------------------ */

Define_Module( ProcessorManager );

void ProcessorManager::initialize()
{
	numOfProcessors = par("numOfProcessors");
	freeProcessors = numOfProcessors;
	
}

void ProcessorManager::activity()
{
	int type;
	cModule *sender;
	cMessage *message;

	while(true)
	{
		message = receive();

		// check if Processor Manager is used
		if (numOfProcessors < 1)
		{
			ignoreProcessorCheck(message);
			continue;
		}

		type = message->kind();
		sender = simulation.module( message->senderModuleId() );

		if (type == PROCMGR_CLAIM_KERNEL)
		{
			kernelClaim(sender, message);
			continue;
		} // end if type == CLAIM_KERNEL

		if (type == PROCMGR_CLAIM_PROCESSOR)
		{
			processorClaim(sender, message);
			continue;
		} // end if type == PROCMGR_CLAIM_PROCESSOR

		if (type == PROCMGR_RELEASE_PROCESSOR)
		{
			freeProcessors++;
			delete message;
			continue;
		} // end if type == PROCMGR_RELEASE_PROCESSOR

		/* no if-statement for PROCMGR_RELEASE_KERNEL 
			or PROCMGR_CLAIM_KERNEL_AGAIN needed;
			always caught from gate "releaseKernelIn" */
			
	} // end while
}

/* 	------------------------------------------------
		Private functions
	------------------------------------------------ */
void ProcessorManager::ignoreProcessorCheck (cMessage *msg)
{
	int type = msg->kind();
	cModule *sender = 
			simulation.module( msg->senderModuleId() );
	
	if (type == PROCMGR_CLAIM_KERNEL || type == PROCMGR_CLAIM_PROCESSOR)
	{
		sendDirect(	new cMessage, 0, sender, "processorManagerIn");
	};

	delete msg;
}

void ProcessorManager::processorClaim( cModule *sender, cMessage *msg)
{
	cMessage *releaseMsg;

	delete msg;

	if (freeProcessors > 0)
	{
		freeProcessors--;
		sendDirect(	new cMessage, 0, 
				sender, "processorManagerIn");
	}

	if (freeProcessors == 0)
	{
		releaseMsg = receiveOn("ReleaseProcessorIn");
		freeProcessors++;
		delete releaseMsg;
	} 

}

void ProcessorManager::kernelClaim( cModule *sender, cMessage *msg)
{

	/* for each claimToken, a release message has to be sent */
	int claimTokenCtr = 1; 
	cMessage *nextMsg; // can be RELEASE_KERNEL or CLAIM_KERNEL_AGAIN

	/* debug output
	ev << "+++ kernel claimed: " 
		<< simulation.module(message->senderModuleId())->fullPath() 
		<< ".\n";
	*/
	delete msg;

	sendDirect(	new cMessage, 0, 
				sender, "processorManagerIn");

	while (claimTokenCtr > 0)
	{
		nextMsg = receiveOn("ReleaseKernelIn");
		if (nextMsg->kind() == PROCMGR_RELEASE_KERNEL)
		{
			claimTokenCtr--;
		} else // nextMsg->kind() == PROCMGR_CLAIM_KERNEL_AGAIN
		{
			claimTokenCtr++;
		}

		/* more debug output
		ev  << "+++ kernel   " 
			<< ((nextMsg->kind() == PROCMGR_RELEASE_KERNEL) 
					: "released: " ? "claimed again: ")
			<< simulation.module(releasemsg->senderModuleId())->fullPath() 
			<< ".\n";
		*/
		delete nextMsg;
	} 
}

