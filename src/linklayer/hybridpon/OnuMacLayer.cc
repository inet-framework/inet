///
/// @file   OnuMacLayer.cc
/// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
/// @date   Jun/30/2009
///
/// @brief  Implements 'OnuMacLayer' class for a Hybrid TDM/WDM-PON ONU.
///
/// @remarks Copyright (C) 2009-2010 Kyeong Soo (Joseph) Kim. All rights reserved.
///
/// @remarks This software is written and distributed under the GNU General
///          Public License Version 2 (http://www.gnu.org/licenses/gpl-2.0.html).
///          You must not remove this notice, or any other, from this software.
///


// For debugging
// #define DEBUG_ONU_MAC_LAYER


#include "OnuMacLayer.h"

// Register modules.
Define_Module(OnuMacLayer);

///
/// Handles an Ethernet frame from UNI (i.e., Ethernet switch).
/// Puts the Ethernet frame into a FIFO, if there is enough space.
/// Otherwise, drops it.
///
/// @param[in] frame	an EtherFrame pointer
///
void OnuMacLayer::handleEthernetFrameFromUni(EtherFrame *frame)
{
#ifdef DEBUG_ONU_MAC_LAYER
	ev << getFullPath() << ": A frame from UNI received" << endl;
#endif

	if (busyQueue + frame->getBitLength() <= queueSize)
	{
		busyQueue += frame->getBitLength();
		queue.insert(frame);

#ifdef DEBUG_ONU_MAC_LAYER
		ev << getFullPath() << ": busyQueue = " << busyQueue << endl;
#endif

	}
	else
	{
		//	         monitor->recordLossStats(frame->getSrcAddress(), frame->getDstnAddress(), frame->getBitLength() );
		delete frame;

#ifdef DEBUG_ONU_MAC_LAYER
		ev << getFullPath() << ": A frame from UNI dropped!" << endl;
#endif
	}
}

///
/// Handles a data PON frame from the PON interface (i.e., OLT).
/// Extracts Ethernet frames from it and sends them to the upper layer (i.e., Ethernet switch).
///
/// @param[in] frame a HybridPonDsDataFrame pointer
///
void OnuMacLayer::handleDataPonFrameFromPon(HybridPonDsDataFrame *frame)
{
#ifdef DEBUG_ONU_MAC_LAYER
	ev << getFullPath() << ": data PON frame received" << endl;
#endif

	cQueue &frameQueue = frame->getEncapsulatedFrames();
	while (!frameQueue.empty())
	{
		EtherFrame *etherFrame =
				check_and_cast<EtherFrame *> (frameQueue.pop());
		send(etherFrame, "ethg$o");
	}
	delete frame;
}

///
/// Handles a grant PON frame from the PON interface (i.e., OLT).
/// Creates a new PON frame, encapsulates Ethernet frames from the FIFO queue in it,
/// and sends it back to the PON interface.
///
/// @param[in] ponFrameDs a HybridPonDsGrantFrame pointer
///
void OnuMacLayer::handleGrantPonFrameFromPon(HybridPonDsGrantFrame *ponFrameDs)
{
#ifdef DEBUG_ONU_MAC_LAYER
	ev << getFullPath() << ": grant PON frame received =" << PonFrameDs->getGrant() << endl;
#endif

	HybridPonUsFrame *ponFrameUs = new HybridPonUsFrame("", HYBRID_PON_FRAME);
	cPacketQueue &frameQueue = ponFrameUs->getEncapsulatedFrames();
	int encapsulatedFramesSize = 0;

	// First, check the grant size for upstream data to determine
	// whether it's for report only (polling) or both report and upstream data.
	int grant = ponFrameDs->getGrant();
	if (grant > 0)
	{
		// It's for both report and upstream data.

		if (queue.empty() != true)
		{
			EtherFrame *frame = check_and_cast<EtherFrame *>(queue.front());
			while (grant >= (frame->getBitLength() + encapsulatedFramesSize))
			{
				// The length of the 1st frame in the FIFO queue is less than or
				// equal to the remaining data grant.

				frame = check_and_cast<EtherFrame *>(queue.pop());
				encapsulatedFramesSize += frame->getBitLength();
				busyQueue -= frame->getBitLength();
				frameQueue.insert(frame);

				if (queue.empty() == true)
				{
					break;
				}
				else
				{
					frame = check_and_cast<EtherFrame *>(queue.front());
				}
			}	// end of while()
		}	// end of if()
	}	// end of if()

	// set report and other fields of the new upstream PON frame.
	ponFrameUs->setReport(busyQueue);
	ponFrameUs->setChannel(ponFrameDs->getChannel());
	ponFrameUs->setBitLength(PREAMBLE_SIZE + DELIMITER_SIZE + REPORT_SIZE
			+ encapsulatedFramesSize);

#ifdef DEBUG_ONU_MAC_LAYER
	ev << getFullPath() << ": PON frame sent upstream with length = " <<
	ponFrameUs->getBitLength() << " and report = " << ponFrameUs->getReport() << endl;
#endif

	// the upstream PON -- made out of the optical CW burst from the OLT -- is transmitted
	// at the start of the optical CW burst (i.e., grant field) in the corresponding downstream
	// PON frame.
	sendDelayed(ponFrameUs, simtime_t(DS_GRANT_OVERHEAD_SIZE/lineRate), "wdmg$o");

	delete ponFrameDs;
}

///
/// Initializes member variables and allocates memory for them, if needed.
///
void OnuMacLayer::initialize()
{
//	channel = (int) getParentModule()->par("lambda");
	cModule *onu = getParentModule();
	cDatarateChannel *chan = check_and_cast<cDatarateChannel *>(onu->gate("phyg$o")->getChannel());
	lineRate = chan->getDatarate();
	queueSize = par("queueSize");
	busyQueue = 0;

	EV << "ONU initialization results:" << endl;
	EV << "- Line rate = " << lineRate << endl;

	// 	monitor = (Monitor *) ( gate("toMonitor")->getPathEndGate()->getOwnerModule() );
	// #ifdef DEBUG_ONU_MAC_LAYER
	// 	ev << getFullPath() << ": monitor pointing to module with id = " << monitor->getId() << endl;
	// #endif
}

///
/// Handles messages by calling appropriate functions for their processing.
/// Starts simulation and run until it will be terminated by kernel.
///
/// @param[in] msg a cMessage pointer
///
void OnuMacLayer::handleMessage(cMessage *msg)
{
	//#ifdef TRACE_MSG
	//	ev.printf();
	//	PrintMsg(*msg);
	//#endif

	if (msg->getArrivalGateId() == findGate("wdmg$i"))
	{
		// PON frame from the WDM layer

		// Check if the message type is correct.
		ASSERT( msg->getKind() == HYBRID_PON_FRAME );

		int frameType =
				(check_and_cast<HybridPonDsFrame *> (msg))->getFrameType();

		//HybridPonFrame *ponFrameFromOlt = (HybridPonFrame *)msg;
		if (frameType == 0)
		{
			// downstream data
			handleDataPonFrameFromPon(check_and_cast<HybridPonDsDataFrame *> (msg));
		}
		else if (frameType == 1)
		{
			// grant
			handleGrantPonFrameFromPon(check_and_cast<HybridPonDsGrantFrame *> (msg));
		}
	}
	else if (msg->getArrivalGateId() == findGate("ethg$i"))
	{
		// Ethernet frame from the upper layer (i.e., Ethernet switch)

		handleEthernetFrameFromUni(check_and_cast<EtherFrame *> (msg));
	}
	else
	{
		// unknown type of message
		error("%s: ERROR: unexpected message kind %d received",
				getFullPath().c_str(), msg->getKind());
	}
}

///
/// Does post-processing.
///
void OnuMacLayer::finish()
{
}
