/***************************************************************************
                          Router_Deq_Hook_Trigger.cc  -  description
                             -------------------
    begin                : Mon Aug 28 2000
    copyright            : (C) 2000 by Dirk Holzhausen
    email                : dholzh@gmx.de
 ***************************************************************************/


#include "router_deq_hook_trigger.h"
#include "hook_types.h"
#include "simulatedINTERNAL.h"

Define_Module ( Router_Deq_Hook_Trigger );


void Router_Deq_Hook_Trigger::activity () {

	// add statistic parameters
	
	long cntIpPackets;
	cntIpPackets = 0;

	
	// get rate
	
	double rate = ( double ) par ( PAR_ROUTER_RATE );			// rate measured in Mbps
	rate *= 1000000.0;	
	
	double waitTime;
	
	long pckEnqueued = 0;
	
	double nextKick = -99999.0;
	long   msg_length = 0;

	
	// watches
		
#ifdef EVAL
	
	WATCH ( pckEnqueued );
	WATCH ( msg_length );
	WATCH ( waitTime );
	WATCH ( nextKick );
	new cWatch ( "data_packets", cntIpPackets );

  long statLBWakeup, statPckEnq, statPckDcd, statPckNo;
	statLBWakeup = statPckEnq = statPckDcd = statPckNo = 0;


	WATCH ( statPckEnq );
	WATCH ( statPckDcd );
	WATCH ( statPckNo );
  WATCH ( statLBWakeup );
	

	long sumLength = 0;
	WATCH ( sumLength );
#endif
	

	// go ahead...
	
	while ( true ) {

		cMessage *rMsg = receive ();			// act as traffic sink


		// statistics...

#ifdef EVAL

		switch ( rMsg->kind () ) {
	
			case PACKET_ENQUEUED:		statPckEnq++; break;
			case DISCARD_PACKET:			statPckDcd++; break;
			case NO_PACKET:					statPckNo++; break;
      case LB_WAKEUP:         statLBWakeup++; break;

		}

#endif
		
		// if this message an PACKET_EENQUEUED confirmation packet, increase counter
		// check also if a wakeup message must be scheduled
		
		if ( rMsg->kind () == PACKET_ENQUEUED ) {
		
		  // if pckEnqueue == 0 then there is no outstanding request message, so schedule one...
		
			if ( pckEnqueued <= 0 ) {
				if ( nextKick <= simTime () )
					scheduleAt ( simTime (), createINTERNALMessage ( LB_WAKEUP ) );
         else
          scheduleAt ( nextKick, createINTERNALMessage ( LB_WAKEUP ) );
			}

			pckEnqueued++;
		}
		
		// if this message is a wakeup, send a request message...
		
		if ( rMsg->kind () == LB_WAKEUP && pckEnqueued > 0 ) 	send ( createINTERNALMessage ( REQUEST_PACKET ), "out" );				
		
				
		// okay, this packet should be a data packet or a no-packet,
		// - if this is a data packet, send it back to router
		// - schedule a new wakeup message if there are enqueued messages...
		
		if ( rMsg->arrivedOn ( "deq_in" ) ) {
			if ( isINTERNALDataPacket ( rMsg ) ) {
				++cntIpPackets;
				--pckEnqueued;
				msg_length = rMsg->length ();
				send ( ( cMessage * ) rMsg->dup(), "pck_out" );

				sumLength += msg_length;

			} else
				msg_length = MIN_FRAG_SIZE_BITS;

			waitTime   = 1.0 * ( ( ( double ) msg_length ) /  rate );
			nextKick   = simTime () + waitTime;		

			if ( pckEnqueued > 0 ) scheduleAt ( nextKick, createINTERNALMessage ( LB_WAKEUP ) );
			
		}
			
		deleteINTERNALMessage ( rMsg );		
	}	
	
}

