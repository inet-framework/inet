///
/// @file   WdmLayer.cc
/// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
/// @date   Jul/9/2012
///
/// @brief  Declares 'WdmLayer' class for a hybrid TDM/WDM-PON ONU.
///
/// @remarks Copyright (C) 2012 Kyeong Soo (Joseph) Kim. All rights reserved.
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

void WdmLayer::initialize()
{
//	// set "flow-through" reception mode for the input gate (phyg$i) for physical layer
//	gate("phyg$i")->setDeliverOnReceptionStart(true);

	// set 'ch' based on either a parameter value
	ch = par("ch").longValue();
//	if (ch < 0)
//	{
//	    // initialize the ONU channel by the gate index of the LambdaSplitter
//	    ch = getParentModule()->gate("phyg$o")->getPathEndGate()->getIndex();
//	}
}

void WdmLayer::handleMessage(cMessage *msg)
{
    Enter_Method("handleMessage()");

	if (msg->getArrivalGateId() == findGate("phyg$i"))
	{   // optical frame from the physical medium (i.e., optical fiber)
		OpticalFrame *opticalFrame = check_and_cast<OpticalFrame *>(msg);
		if (ch != opticalFrame->getLambda())
        {
            error("The channle index %d of the incoming optical frame does not match with %d of the WDM layer",
                opticalFrame->getLambda(), ch);
        }

#ifdef DEBUG_WDM_LAYER
		ev << getFullPath() << ": Optical frame with a WDM channel = " << ch << endl;
#endif

		// decapsulate a packet (e.g., Ethernet or PON frame) and send it to the upper layer
//		cPacket *frame = check_and_cast<cPacket *> (
//				opticalFrame->decapsulate());
//		frame->setChannel(ch);
		send(opticalFrame->decapsulate(), "pong$o");
		delete opticalFrame;
	}
	else
	{   // packet from the upper layer (i.e., Ethernet or PON layer)

//		HybridPonUsFrame *frame = check_and_cast<HybridPonUsFrame *>(msg);
//		int ch = frame->getChannel();
//
//#ifdef DEBUG_WDM_LAYER
//		ev << getFullPath() << ": PON frame with a channel index = " << ch << endl;
//#endif

		// encapsulate a packet into an optical frame and send it to the PON interface
	    cPacket *pkt = check_and_cast<cPacket *>(msg);
		OpticalFrame *opticalFrame = new OpticalFrame();
		opticalFrame->setLambda(ch);
		opticalFrame->encapsulate(pkt);
		send(opticalFrame, "phyg$o");
	}
}

void WdmLayer::finish()
{
}
