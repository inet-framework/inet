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

/* 	------------------------------------------------
	file: ProcessorManager.h
	Purpose: Simulate processor usage of an IPNode
	relies on direct send in and out
	set numOfProcessors=1 for single processor node
	set numOfProcessors=0 to disable processor check
	author: Jochen Reber
	------------------------------------------------ */

#ifndef __PROCESSORMANAGER_H__
#define __PROCESSORMANAGER_H__

#include <omnetpp.h>
#include "basic_consts.h"


/*  ------------------------------------------------
        Constants
    ------------------------------------------------ */

const int   PROCMGR_CLAIM_KERNEL        = 1,
            PROCMGR_CLAIM_PROCESSOR     = 2,
            PROCMGR_RELEASE_KERNEL      = 3,
            PROCMGR_RELEASE_PROCESSOR   = 4,   
            PROCMGR_CLAIM_KERNEL_AGAIN  = 5;

/* 	------------------------------------------------
		Main Class: ProcessorManager
	------------------------------------------------ */
class ProcessorManager: public cSimpleModule
{
private:
	int numOfProcessors;
	int freeProcessors;

	void ignoreProcessorCheck(cMessage *);
	void kernelClaim(cModule *sender, cMessage *msg);
	void processorClaim(cModule *sender, cMessage *msg);
public:
    Module_Class_Members(ProcessorManager, 
				cSimpleModule, ACTIVITY_STACK_SIZE);

	virtual void initialize();
	virtual void activity();

};

#endif
