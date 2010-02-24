///
/// @file   HybridPonMac.cc
/// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
/// @date   Jun/30/2009
///
/// @brief  Implements 'HybridPonMac' class for a Hybrid TDM/WDM-PON ONU.
///
/// @remarks Copyright (C) 2009-2010 Kyeong Soo (Joseph) Kim. All rights reserved.
///
/// @remarks This software is written and distributed under the GNU General
///          Public License Version 2 (http://www.gnu.org/licenses/gpl-2.0.html).
///          You must not remove this notice, or any other, from this software.
///


// For debugging
// #define DEBUG_HYBRID_PON_MAC


#include "HybridPonMac.h"

// Register modules.
Define_Module(HybridPonMac)

///
/// Handle an Ethernet frame from UNIs.
/// Put the Ethernet frame into a FIFO, if there is enough space.
/// Otherwise, drop it.
///
/// @param[in] frame a EtherFrame pointer
///
void HybridPonMac::handleEthernetFrameFromUni(EtherFrame *frame)
{
#ifdef DEBUG_HYBRID_PON_MAC
	ev << getFullPath() << ": A frame from UNI received" << endl;
#endif

	if (busyQueue + frame->getBitLength() <= queueSize)
	{
		busyQueue += frame->getBitLength();
		queue.insert(frame);

#ifdef DEBUG_HYBRID_PON_MAC
		ev << getFullPath() << ": busyQueue = " << busyQueue << endl;
#endif

	}
	else
	{
		//	         monitor->recordLossStats(frame->getSrcAddress(), frame->getDstnAddress(), frame->getBitLength() );
		delete frame;

#ifdef DEBUG_HYBRID_PON_MAC
		ev << getFullPath() << ": A frame from UNI dropped!" << endl;
#endif
	}
}

///
/// Handle a data PON frame from the PON I/F.
/// Extract Ethernet frames from it and send them to the upper layer (i.e., Ethernet switch).
///
/// @param[in] frame a HybridPonDsDataFrame pointer
///
void HybridPonMac::handleDataFromPon(HybridPonDsDataFrame *frame)
{
#ifdef DEBUG_HYBRID_PON_MAC
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
/// Handle a grant PON frame from the PON I/F.
/// Creates a new PON frame, encapsulate Ethernet frames from the FIFO queue in it,
/// and send it back to the PON I/F.
///
/// @param[in] frame a HybridPonDsGrantFrame pointer
///
void HybridPonMac::handleGrantFromPon(HybridPonDsGrantFrame *ponFrameDs)
{
#ifdef DEBUG_HYBRID_PON_MAC
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

#ifdef DEBUG_HYBRID_PON_MAC
	ev << getFullPath() << ": PON frame sent upstream with length = " <<
	ponFrameUs->getBitLength() << " and report = " << ponFrameUs->getReport() << endl;
#endif

	// the upstream PON -- made out of the optical CW burst from the OLT -- is transmitted
	// at the start of the optical CW burst (i.e., grant field) in the corresponding downstream
	// PON frame.
	sendDelayed(ponFrameUs, simtime_t(DS_GRANT_OVERHEAD_SIZE/BITRATE), "wdmg$o");

	delete ponFrameDs;
}

///
/// Initialize member variables and allocate memory for them, if needed.
///
void HybridPonMac::initialize()
{
//	channel = (int) getParentModule()->par("lambda");
	queueSize = par("queueSize");
	busyQueue = 0;

	// 	monitor = (Monitor *) ( gate("toMonitor")->getPathEndGate()->getOwnerModule() );
	// #ifdef DEBUG_HYBRID_PON_MAC
	// 	ev << getFullPath() << ": monitor pointing to module with id = " << monitor->getId() << endl;
	// #endif
}

///
/// Handle messages by calling appropriate functions for their
/// processing. Start simulation and run until it will be terminated by
/// kernel.
///
/// @param[in] msg a cMessage pointer
///
void HybridPonMac::handleMessage(cMessage *msg)
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
			handleDataFromPon(check_and_cast<HybridPonDsDataFrame *> (msg));
		}
		else if (frameType == 1)
		{
			// grant
			handleGrantFromPon(check_and_cast<HybridPonDsGrantFrame *> (msg));
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
/// Do post-processing.
///
void HybridPonMac::finish()
{
}
