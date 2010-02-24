// $Id$
//------------------------------------------------------------------------------
//	LambdaSplitter.cc --
//
//	This file implements 'LambdaSplitter' class, modelling an AWG in the path
//  of PONs.
//
//	Copyright (C) 2009-2010 Kyeong Soo (Joseph) Kim
//------------------------------------------------------------------------------


// #define DEBUG_LAMBDA_SPLITTER


#include <stdlib.h>
#include "LambdaSplitter.h"


// Register modules.
Define_Module(LambdaSplitter)


void LambdaSplitter::initialize()
{
	numPorts = gateSize("demuxGate");
}


void LambdaSplitter::handleMessage(cMessage *msg)
{
	OpticalFrame *opticalFrame = (OpticalFrame *) msg;

#ifdef DEBUG_LAMBDA_SPLITTER
	ev << getFullPath() << ": handleMessage called" << endl;
#endif

	if ( msg->getArrivalGateId() == findGate("muxGate") ) {
        // This is a downstream frame from the OLT to an ONU.

#ifdef DEBUG_LAMBDA_SPLITTER
		ev << getFullPath() << ": Downstream opticalFrame for onu[" << opticalFrame->lambda() << "]" << endl;
#endif

		int i=opticalFrame->getLambda();
		//ownership problem here or up there?
		send(opticalFrame, "demuxGate", i);

// #ifdef DEBUG_LAMBDA_SPLITTER
// 		ev << getFullPath() << ": scheduled to be sent at " << simTime() + delay << endl;
// #endif
	}
	// bug found here: msg already has a new arrivalGateId.  Should do this only else
	else {
        // This is an upstream frame from an ONU to the OLT.

		for (int i=0; i<=numPorts; i++) {
			if ( msg->getArrivalGateId() == findGate("demuxGate",i) ) {

#ifdef DEBUG_LAMBDA_SPLITTER
				ev << "hello: " << msg->getArrivalGateId() << ", " << findGate("demuxGate",i)
				   << ", " << findGate("muxGate") << endl;
				ev << getFullPath() << ": Upstream opticalFrame from onu[" << i << "]" << endl;
#endif

				if ( opticalFrame->getLambda() == i) {
// 					sendDelayed (opticalFrame, RTT[i]/2.0, "muxGate");
                    send(opticalFrame, "muxGate");

#ifdef DEBUG_LAMBDA_SPLITTER
                    ev << getFullPath() << ": scheduled to be sent at " << simTime() + delay << endl;
#endif
				}
				else {
					ev << getFullPath() << ": error: received frame from ONU[" << i
                       << "] on wavelength " << opticalFrame->getLambda() << endl;
                    exit(1);
				}
			}
		}   // end of for loop
	}
}


void LambdaSplitter::finish()
{
}
