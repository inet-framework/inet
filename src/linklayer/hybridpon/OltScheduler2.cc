///
/// @file   OltScheduler2.cc
/// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
/// @date   Jul/09/2012
///
/// @brief  Implements 'OltScheduler2' class for hybrid TDM/WDM-PON OLT.
///
/// @remarks Copyright (C) 2012 Kyeong Soo (Joseph) Kim. All rights reserved.
///
/// @remarks This software is written and distributed under the GNU General
///          Public License Version 2 (http://www.gnu.org/licenses/gpl-2.0.html).
///          You must not remove this notice, or any other, from this software.
///


// for debugging
//#define DEBUG_OLT_SCHEDULER2


#include "OltScheduler2.h"

OltScheduler2::OltScheduler2()
{
}

OltScheduler2::~OltScheduler2()
{
    for (int i = 0; i < numOnus; i++)
    {
        cancelAndDelete(releaseTxMsg[i]);
    }
}

//------------------------------------------------------------------------------
//	Misc. functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//	Event handling functions
//------------------------------------------------------------------------------

///
/// Initializes member variables & activities and allocates memories
/// for them, if needed.
///
void OltScheduler2::initialize(void)
{
    cModule *olt = getParentModule();

    // initialize OLT NED parameters
	numTransmitters = olt->par("numTransmitters").longValue();

	// initialize OltScheduler2 NED parameters

    // initialize configuration and status variables
	cDatarateChannel *chan = check_and_cast<cDatarateChannel *>(olt->gate("phyg$o", 0)->getChannel());
	lineRate = chan->getDatarate();
	numOnus = olt->gateSize("phyg$o");   ///< = number of ONUs (i.e., WDM channels)
	numTxsAvailable = numTransmitters;

    // initialize self messagess
	releaseTxMsg.assign(numOnus, (HybridPonMessage *) NULL);
    for (int i = 0; i < numOnus; i++)
    {
        releaseTxMsg[i] = new HybridPonMessage("Release a transmitter", RELEASE_TX);
        releaseTxMsg[i]->setOnuIdx(i);
    }
}

///
/// Handles a message by calling a function for its processing.
/// A function is called based on the message type and the port it is received from.
///
/// @param[in] msg a cMessage pointer
///
void OltScheduler2::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage() == true)
    {   // Self message to indicate an event.
        switch (msg->getKind())
        {
            case RELEASE_TX:
                handleEndTxMsg(check_and_cast<HybridPonMessage *>(msg));
                break;
            default:
                error("%s: Unexpected message type: %d", getFullPath().c_str(), msg->getKind());
                break;
        } // end of switch()
    }   // end of if()
    else
    {   // Ethernet frame from an external interface
        std::string inGate = msg->getArrivalGate()->getFullName();
#ifdef DEBUG_OLT_SCHEDULER2
        ev << getFullPath() << ": A frame from " << inGate << " received" << endl;
#endif
        if (inGate.compare(0, 6, "ethg$i") == 0)
        {   // from the upper layer (i.e., Ethernet switch)
            handleEthernetFrameFromSni(check_and_cast<EtherFrame *>(msg));
        }
        else if (inGate.compare(0, 6, "wdmg$i") == 0)
        {   // from the lower layer (i.e., WDM layer)
            int ch = msg->getArrivalGate()->getIndex(); // channel index = gate index
            send(msg, "ethg$o", ch);
        }
        else
        {   // unknown message
            error("%s: Unknown message received from %s", getFullPath().c_str(), inGate.c_str());
        }
    } // end of else
}

///
/// Does post processing and deallocates memories manually allocated
/// for member variables.
///
void OltScheduler2::finish(void)
{
}
