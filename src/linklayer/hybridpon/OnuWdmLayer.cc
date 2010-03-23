///
/// @file   OnuWdmLayer.cc
/// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
/// @date   Feb/22/2010
///
/// @brief  Declares 'OnuWdmLayer' class for a hybrid TDM/WDM-PON ONU.
///
/// @remarks Copyright (C) 2010 Kyeong Soo (Joseph) Kim. All rights reserved.
///
/// @remarks This software is written and distributed under the GNU General
///          Public License Version 2 (http://www.gnu.org/licenses/gpl-2.0.html).
///          You must not remove this notice, or any other, from this software.
///


// #define DEBUG_WDM_LAYER

#include <stdlib.h>
#include "OnuWdmLayer.h"

// Register modules.
Define_Module(OnuWdmLayer)

void OnuWdmLayer::initialize()
{
	// set "flow-through" reception mode for the input gate (phyg$i) for physical layer
	gate("phyg$i")->setDeliverOnReceptionStart(true);
}

void OnuWdmLayer::handleMessage(cMessage *msg)
{
#ifdef DEBUG_WDM_LAYER
	ev << getFullPath() << ": handleMessage called" << endl;
#endif

	if (msg->getArrivalGateId() == findGate("phyg$i"))
	{
		// optical frame from the physical medium (i.e., optical fiber)

		OpticalFrame *opticalFrame = check_and_cast<OpticalFrame *> (msg);
		int ch = opticalFrame->getLambda();

#ifdef DEBUG_WDM_LAYER
		ev << getFullPath() << ": Optical frame with a WDM channel = " << ch << endl;
#endif

		// decapsulate a PON frame and send it to the upper layer
		HybridPonDsFrame *frame = check_and_cast<HybridPonDsFrame *> (
				opticalFrame->decapsulate());
		frame->setChannel(ch);
		send(frame, "pong$o");
		delete opticalFrame;
	}
	else
	{
		// PON frame from the upper layer (i.e., PON layer)

		HybridPonUsFrame *frame = check_and_cast<HybridPonUsFrame *> (msg);
		int ch = frame->getChannel();

#ifdef DEBUG_WDM_LAYER
		ev << getFullPath() << ": PON frame with a channel index = " << ch << endl;
#endif

		// encapsulate a PON frame into an optical frame and send it to the PON interface
		OpticalFrame *opticalFrame = new OpticalFrame();
		opticalFrame->setLambda(ch);
		opticalFrame->encapsulate(frame);
		send(opticalFrame, "phyg$o");
	}
}

void OnuWdmLayer::finish()
{
}
