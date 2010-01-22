///
/// @file   WdmLayer.cc
/// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
/// @date   Jan/21/2010
///
/// @brief  Declares 'WdmLayer' class for a hybrid TDM/WDM-PON.
///
/// @remarks Copyright (C) 2009-2010 Kyeong Soo (Joseph) Kim. All rights reserved.
///
/// @remarks This software is written and distributed under the GNU General
///          Public License Version 2 (http://www.gnu.org/licenses/gpl-2.0.html).
///          You must not remove this notice, or any other, from this software.
///


// #define DEBUG_WDM_LAYER


#include <stdlib.h>
#include "WdmLayer.h"

// Register modules.
Define_Module(WdmLayer)

void WdmLayer::initialize() {
	inBaseId = getBaseId("demuxg$i");
	outBaseId = getBaseId("demuxg$o");
	//	gateSize = gateSize("demuxg$i");
}

void WdmLayer::handleMessage(cMessage *msg) {
#ifdef DEBUG_WDM_LAYER
	ev << getFullPath() << ": handleMessage called" << endl;
#endif

	if (msg->getArrivalGateId() == findGate("muxg$i")) {
		// This is the optical frame from the MUX gate (i.e., from the optical fiber).

		OpticalFrame *opticalFrame = check_and_cast<OpticalFrame *> (msg);
		int i = opticalFrame->getLambda();

#ifdef DEBUG_WDM_LAYER
		ev << getFullPath() << ": optical frame with a channel index = " << i << endl;
#endif

		// Decapsulate a frame and send it to the upper layer
		cPacket *frame = opticalFrame->decapsulate();
		send(frame, "demuxg", outBaseId + i); //ownership problem here or up there?
		delete opticalFrame;
	} else {
		// This is the frame from the DEMUX gate (i.e., from the upper layer).

		int i = findGate("demuxg$i") - inBaseId; // get channel index
		OpticalFrame *opticalFrame = new OpticalFrame();
		opticalFrame->setLambda(i);
		opticalFrame->encapsulate(msg);
		send(opticalFrame, "muxg");

		//		for (int i=0; i<=gateSize; i++) {
		//			if ( msg->getArrivalGateId() == findGate("demuxGate",i) ) {
		//
		//#ifdef DEBUG_WDM_LAYER
		//				ev << "hello: " << msg->getArrivalGateId() << ", " << findGate("demuxGate",i)
		//				   << ", " << findGate("muxGate") << endl;
		//				ev << getFullPath() << ": Upstream opticalFrame from onu[" << i << "]" << endl;
		//#endif
		//
		//				if ( opticalFrame->getLambda() == i) {
		//                    send(opticalFrame, "muxg");
		//				}
		//				else {
		//					ev << getFullPath() << ": error: received frame from ONU[" << i
		//                       << "] on wavelength " << opticalFrame->getLambda() << endl;
		//                    exit(1);
		//				}
		//			}
	} // end of for loop
}

void WdmLayer::finish() {
}
