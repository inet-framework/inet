///
/// @file   LambdaSplitter.cc
/// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
/// @date   2010-02-25
///
/// @brief  Implements LambdaSplitter class, modelling AWG for a hybrid
///			TDM/WDM-PON.
///
/// @remarks Copyright (C) 2010 Kyeong Soo (Joseph) Kim. All rights reserved.
///
/// @remarks This software is written and distributed under the GNU General
///          Public License Version 2 (http://www.gnu.org/licenses/gpl-2.0.html).
///          You must not remove this notice, or any other, from this software.
///


// #define DEBUG_LAMBDA_SPLITTER


#include <stdlib.h>
#include "LambdaSplitter.h"

// Register modules.
Define_Module(LambdaSplitter)

void LambdaSplitter::initialize()
{
	int numPorts = this->gateSize("demuxg$i");

	// set "flow-through" reception mode for all input gates
	for (int i = 0; i < numPorts; i++)
	{
		gate("demuxg$i", i)->setDeliverOnReceptionStart(true);
		gate("muxg$i", i)->setDeliverOnReceptionStart(true);
	}
}

void LambdaSplitter::handleMessage(cMessage *msg)
{
	// get the full name of arrival gate.
	std::string inGate = msg->getArrivalGate()->getFullName();

	OpticalFrame *opticalFrame = check_and_cast<OpticalFrame *> (msg);
	int ch = opticalFrame->getLambda();

	if (inGate.compare(0, 6, "muxg$i") == 0)
	{
		// optical frame from the MUX gate (i.e., the OLT)

#ifdef DEBUG_LAMBDA_SPLITTER
		EV << getFullPath() << ": OpticalFrame from the OLT with a WDM channel = " << ch << endl;
#endif

		send(opticalFrame, "demuxg$o", ch);
	}
	else
	{
		// optical frame from the DEMUX gate (i.e., the ONU)

#ifdef DEBUG_LAMBDA_SPLITTER
		EV << getFullPath() << ": OpticalFrame from the ONU with a WDM channel = " << ch << endl;
#endif

		send(opticalFrame, "muxg$o", ch);
	}
}

void LambdaSplitter::finish()
{
}
