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

/*  ---------------------------------------------------------
    file: ProcessorAccess.cc
    Purpose: Implementation of ProcessAccess base class;
             gives access to the ProcessorManager
    comment: ProcessorManager-Module needs to be named
	     "processorManager"
	     all Modules using the processor manager need
	     an input gate called "processorManagerIn"
    --------------------------------------------------------- */


#include "ProcessorManager.h"
#include "ProcessorAccess.h"

Define_Module( ProcessorAccess );

void ProcessorAccess::initialize()
{
	cObject *foundmod;
	cModule *curmod = this;

	processorManager = NULL;
	// find Processor Manager
	for (curmod = parentModule(); curmod != NULL; 
				curmod = curmod->parentModule())
	{
		if ((foundmod = curmod->findObject("processorManager", false)) 
				!= NULL)
		{
			/* debugging output
			ev << "ProcMangr Access init " << fullPath()
				<< ": ProcMangr Module found.\n";
			*/
			processorManager = (cSimpleModule *)foundmod;
			return;
		}
	}
	ev << "ProcMangr Access init " << fullPath() 
		<< ": ProcMangr Module NOT found.\n";

}
/*  ----------------------------------------------------------
        Procted Functions: Processor Manager
    ----------------------------------------------------------  */
void ProcessorAccess::releaseKernel()
{
	if (processorManager != NULL)
	{
    	sendDirect( 
			new cMessage("Release Kernel", PROCMGR_RELEASE_KERNEL), 
			0, processorManager, "ReleaseKernelIn");
	} else
	{
		writeErrorMessage("releaseKernel");
	}
}

void ProcessorAccess::claimKernel()
{
	cMessage *confMsg;

	if (processorManager != NULL)
	{
    	sendDirect( new cMessage("Claim Kernel", PROCMGR_CLAIM_KERNEL),
			0, processorManager, "ClaimKernelIn");
    	confMsg = receiveNewOn("processorManagerIn");
		delete confMsg;
	} else
	{
		writeErrorMessage("claimKernel");
	}
}

void ProcessorAccess::claimKernelAgain()
{
	if (processorManager != NULL)
	{
		/* needs to be sent to releaseKernelIn-gate in order
		   to allow the ProcessorManager to process message! */
    	sendDirect( new cMessage("Claim Kernel", PROCMGR_CLAIM_KERNEL_AGAIN),
			0, processorManager, "ReleaseKernelIn");
	} else
	{
		writeErrorMessage("claimKernelAgain");
	}

}

void ProcessorAccess::releaseProcessor()
{
	if (processorManager != NULL)
	{
    	sendDirect( 
			new cMessage("Release Processor", PROCMGR_RELEASE_PROCESSOR),
			0, processorManager, "ReleaseProcessorIn");
	} else
	{
		writeErrorMessage("releaseProcessor");
	}
}

void ProcessorAccess::claimProcessor()
{
	cMessage *confMsg;

	if (processorManager != NULL)
	{
    	sendDirect(
				new cMessage("Claim Processor", PROCMGR_CLAIM_PROCESSOR),
			0, processorManager, "ClaimProcessorIn");
    	confMsg = receiveNewOn("processorManagerIn");
		delete confMsg;
	} else
	{
		writeErrorMessage("claimProcessor");
	}
}

void ProcessorAccess::writeErrorMessage(const char *methodName)
{
	ev << "ProcMangr Access Error " << fullPath() << " "
			<< methodName << ": ProcMangr Module not found.\n";
}

