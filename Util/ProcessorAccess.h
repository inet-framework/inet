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

/*  ---------------------------------------------------------
    file: ProcessorAccess.h
    Purpose: Header file for ProcessorAccess base class;
		gives access to the ProcessorManager
    --------------------------------------------------------- */

#ifndef __PROCESSOR_ACCESS_H__
#define __PROCESSOR_ACCESS_H__

#include <omnetpp.h>

class ProcessorAccess: public cSimpleModule
{
private:
	cSimpleModule *processorManager;

protected:
    void releaseKernel();
    void claimKernel();
	/* claimKernelAgain() can be
		called after the Kernel has been
		claimed, so that multiple releaseKernel()-
		calls are required to release the Kernel
		finally ;
		it can be used to spawn processes inside
		atomic processes */
	void claimKernelAgain();
	void claimProcessor();
	void releaseProcessor();

	void writeErrorMessage(const char *);

public:
    Module_Class_Members(ProcessorAccess, cSimpleModule, 0);

	virtual void initialize();
	
};

#endif

