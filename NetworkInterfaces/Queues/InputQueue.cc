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
	file: InputQueue.cc
	Purpose: Implementation of L2 InputQueue
	Responsibilities: 
	author: Jochen Reber
*/
	
#include <omnetpp.h>

#include "InputQueue.h"
#include "IPDatagram.h"

Define_Module( InputQueue );


void InputQueue::initialize()
{
	ProcessorAccess::initialize();

	delay = 0; //par("procdelay");
}

void InputQueue::activity()
{
	
	IPDatagram *datagram;

	while(true)
	{
		/* receive packets from all network
			interfaces */
		datagram = (IPDatagram *) receive(); 

		claimKernel();
		wait(delay);
		// gates to processor Manager and IP layer come first
		// count switches from starting at 1 to starting at 0
		//datagram->setInputPort(datagram->arrivalGate()->id() - 3);
		datagram->setInputPort(datagram->arrivalGate()->index());
			
		send(datagram, "toIP");
		releaseKernel();
	}
}

