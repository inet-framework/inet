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
#include "FIFO_Dequeue.h"
#include "FIFO_Queue.h"
#include "any_queue.h"
#include "hook_types.h"
#include "simulatedINTERNAL.h"


Define_Module ( FIFO_Dequeue );



/*
  initialize: use multi-stage initialization to make sure that FIFO_Queue has
  been initialized first
*/

void FIFO_Dequeue::initialize( int stage ) {

	if ( stage > 0 ) {
	
		// find queue pointer...
		
		const char *qName = par ( PAR_QUEUE_NAME );
		fifo_queue = ( FIFO_Queue * ) findINTERNALQueue ( this, qName );

		if ( !fifo_queue ) error ( "Queue %s not found in module %s", qName, fullPath () );

		cntSentPackets = cntNoPackets = 0;

#ifdef EVAL
		ev << "Module " << fullName () << ": Queue "  << qName << " " << ( fifo_queue ? "found" : "not found" ) << endl;	

		new cWatch ( "sent packets", cntSentPackets );
		new cWatch ( "no packets", cntNoPackets );
#endif
		
	}
}


void FIFO_Dequeue::handleMessage ( cMessage *msg ) {

		// delete request message to save memory
		
    deleteINTERNALMessage ( msg );    //delete msg;


		if ( fifo_queue->isEmpty() ) {
			/*
			cMessage *No_Packet = new cMessage ("No_Packet", 0);
			send (No_Packet, "to_deq_desc");
			*/
			
			cMessage *rMsg = createINTERNALMessage ( NO_PACKET );   //new cMessage ( "No_Packet", NO_PACKET );
			send ( rMsg, "to_deq_desc" );

			cntNoPackets++;
			
		} else{
		
			/*
			cMessage *Yes_Packet = new cMessage ("Yes_Packet", 1);
			cMessage *Message = fifo_queue->Pop();
			send(Yes_Packet, "to_deq_desc");
			send(Message, "out");
			*/
			
			// packet goes back to deq hook as implicit yes-packet
			
			cMessage *rMsg = fifo_queue->pop ();
			send ( rMsg, "to_deq_desc" );

			cntSentPackets++;
		}
	
		
}


