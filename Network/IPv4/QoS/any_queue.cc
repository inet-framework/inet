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

/***************************************************************************
                          ds_queue.cc  -  description
                             -------------------
    begin                : Thu Aug 24 2000
    author               : Dirk Holzhausen
    email                : dholzh@gmx.de
 ***************************************************************************/


#include "any_queue.h"
#include "hook_types.h"
#include "simulatedINTERNAL.h"



ANY_Queue::ANY_Queue( const char *name, cModule *parentmodule, unsigned stacksize) :
	cSimpleModule ( name, parentmodule, stacksize ) {
}


/*
		initialize:
			use multi-stage initialization to make sure that this class is created before
			enqueue/dequeue modules try to get the pointer of this class
*/


void ANY_Queue::initialize ( int stage ) {

	if ( stage == 0 ) {

		// export queue pointer to "queue_pointer"
	
		if ( findPar ( PAR_QUEUE_POINTER ) < 0 ) addPar ( PAR_QUEUE_POINTER );
		par ( PAR_QUEUE_POINTER ).setPointerValue ( this );

	
		
		// call subclass intialization
		
		startup ();
	}
}

