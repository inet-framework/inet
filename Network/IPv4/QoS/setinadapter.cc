/***************************************************************************
                          setinadapter.cc  -  description
                             -------------------
    begin                : Fri Sep 8 2000
    copyright            : (C) 2000 by Dirk Holzhausen
    email                : dholzh@gmx.de
 ***************************************************************************/


#include "setinadapter.h"
#include "simulatedINTERNAL.h"
#include "omnetpp.h"


Define_Module ( SetInAdapter );

void SetInAdapter::initialize () {
	inAdapter  = par ( PAR_IP_IN_ADAPTER );
}


void SetInAdapter::handleMessage ( cMessage *msg ) {

	if ( msg->findPar ( PAR_IP_IN_ADAPTER ) < 0 ) msg->addPar ( PAR_IP_IN_ADAPTER );				// parameter does not exist -> add!

	msg->par ( PAR_IP_IN_ADAPTER ) = inAdapter;
	
	send ( msg, "out" );

}
