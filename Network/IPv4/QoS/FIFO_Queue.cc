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
                          FIFO_Queue.cc  -  description
                             -------------------
    begin                : Wed Jul 26 2000
    author               : Dirk A. Holzhausen
    email                : dirk.holzhausen@gmx.de

	
    based on FIFO_Queue2

    Änderungen:
       26.07.2000/dh:      Var. FIFO_Queue in FIFOQueue umbenannt,
                           wegen Compilerfehler
 ***************************************************************************/

#include "FIFO_Queue.h"
#include "omnetpp.h"
#include "simulatedINTERNAL.h"

Define_Module( FIFO_Queue );


FIFO_Queue::FIFO_Queue ( const char *name, cModule *parentmodule, unsigned stacksize) :
	ANY_Queue ( name, parentmodule, stacksize ) {
	
}


/*
		startup:
			initialization method called by initialize from basic class
*/


void FIFO_Queue::startup() {
	
/*
		addPar ( "queue_len" );
		addPar ( "packets_enqueued" );
		addPar ( "packets_dequeued" );
		
		par ( "packets_enqueued" ) = pckEnqCnt = 0;
		par ( "packets_dequeued" ) = pckDeqCnt = 0;
		par ( "queue_len" ) = queue_length = 0;
*/

		pckEnqCnt =  pckDeqCnt = queue_length = 0;
		
#ifdef EVAL

		new cWatch ( "packets enqueued", pckEnqCnt );
		new cWatch ( "packets dequeued", pckDeqCnt );
		new cWatch ( "queue length", queue_length );

#endif

		
		
		max_length = par( PAR_QUEUE_MAX_LEN );
		//max_length *= 8;												// queue length in byte -> calculate bit length

		
			
		// create Fifo Queue
				
		fifo_queue = new cQueue("FIFO_Queue");
	
		
		// log message
		
		ev << "Module " << fullName () << ": Queue created!\n";
		
}


bool FIFO_Queue::insert(cMessage * msg)
{

	bool insertOk = false;
	
	if ( isINTERNALDataPacket ( msg ) ) {

		if ((msg->length() + queue_length) > max_length) 	{

			ev << "Queue " << fullName() << " is full " << " (" << queue_length << " bits) (" << fullPath () << ")" << endl;
			insertOk = false;

		}	else {
		
			fifo_queue->insert(msg);
			queue_length += msg->length();
			++pckEnqCnt;
			//par("queue_len") = queue_length;
			//par ( "packets_enqueued" ) = pckEnqCnt;
			insertOk = true;
		}
	}
	
	return insertOk;
}

cMessage *FIFO_Queue::pop()
{
	cMessage	*msg = (cMessage *) fifo_queue->pop();
	queue_length -= msg->length();
	++pckDeqCnt;
	//par("queue_len") = queue_length;
	//par ( "packets_dequeued" ) =  pckDeqCnt;
	
	return msg;
}

cMessage *FIFO_Queue::tail()
{
	return (cMessage *) fifo_queue->tail();
}


cMessage *FIFO_Queue::head()
{
	cMessage *msg = (cMessage *) fifo_queue->head();

	return msg;
}


bool FIFO_Queue::isEmpty()
{
	return fifo_queue->empty();
}

long FIFO_Queue::length() const
{
	return queue_length;
}	

