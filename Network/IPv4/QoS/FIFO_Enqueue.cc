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

#include <omnetpp.h>
#include "FIFO_Enqueue.h"
#include "FIFO_Queue.h"
#include "any_queue.h"
#include "hook_types.h"
#include "simulatedINTERNAL.h"

Define_Module ( FIFO_Enqueue );

/*
  initialize: use multi-stage initialization to make sure that FIFO_Queue has
  been initialized first

*/


void FIFO_Enqueue::initialize( int stage ) {

	if ( stage > 0 ) {
	
		// find queue pointer...
	
		const char *qName = par ( PAR_QUEUE_NAME );
		fifo_queue = ( FIFO_Queue * ) findINTERNALQueue ( this, qName );

		if ( !fifo_queue )error ( "Queue %s not found in module %s", qName, fullPath () );



#ifdef EVAL
		ev << "Module " << fullName () << ": Queue "  << qName << " " << ( fifo_queue ? "found" : "not found" ) << endl;	

		cntEnqueued = cntDiscard = 0;
		new cWatch ( "enqueued packets", cntEnqueued );
		new cWatch ( "discarded packets", cntDiscard );
#endif
	
	}
	
}


void FIFO_Enqueue::handleMessage ( cMessage *msg ) {

		bool ok = fifo_queue->insert(msg);
		
		if ( ok )
			send ( createINTERNALMessage ( PACKET_ENQUEUED ), "to_enq_desc" ); //send ( new cMessage ( "Packet_Enqueued", PACKET_ENQUEUED ), "to_enq_desc" );
		 else {
			msg->setName ( "Discard_Packet" );
			msg->setKind ( DISCARD_PACKET );			// send message back as discard packet
			send ( msg, "to_enq_desc" );
		}

#ifdef EVAL
		if ( ok )
			cntEnqueued++;
     else
			cntDiscard++;
#endif
		
}


