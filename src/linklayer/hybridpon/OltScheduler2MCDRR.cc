///
/// @file   OltScheduler2MCDRR.cc
/// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
/// @date   2012-07-09
///
/// @brief  Implements 'OltScheduler2MCDRR' class for a hybrid TDM/WDM-PON OLT.
///
/// @note
/// This file implements an 'OltScheduler2MCDRR' class for the 'Multi-Channel
/// Deficit Round-Robin' scheduling algorithm proposed in [1].
///
/// @par References:
/// <ol>
///	<li>
/// Mithileysh Sathiyanarayanan and Kyeong Soo Kim, &quot;Multi-channel deficit
/// round-robin scheduling algorithms for hybrid TDM/WDM-PON,&quot; FOAN 2012, 2012.
/// </li>
/// </ol>
///
/// @remarks Copyright (C) 2012 Kyeong Soo (Joseph) Kim. All rights reserved.
///
/// @remarks This software is written and distributed under the GNU General
///          Public License Version 2 (http://www.gnu.org/licenses/gpl-2.0.html).
///          You must not remove this notice, or any other, from this software.
///


// For debugging
// #define DEBUG_OLT_SCHEDULER2MCDRR


#include "OltScheduler2.h"

// Register module.
Define_Module(OltScheduler2MCDRR);

OltScheduler2MCDRR::~OltScheduler2MCDRR()
{
    for (int i = 0; i < numOnus; i++)
    {
        delete queues[i];
    }
}

//------------------------------------------------------------------------------
//	Misc. functions
//------------------------------------------------------------------------------

bool OltScheduler2MCDRR::enqueue(EtherFrame *frame)
{
    int ch = frame->getArrivalGate()->getIndex();
    cQueue *queue = queues[ch];
    if (frameCapacity && queue->length() >= frameCapacity)
    {
        EV << "Queue[" << ch << "] full, dropping packet.\n";
        delete frame;
        numQueueDropped[ch]++;
        return true;
    }
    else
    {
        queue->insert(frame);
        return false;
    }
}

EtherFrame *OltScheduler2MCDRR::dequeue()
{
    bool found = false;
    int startQueueIndex = (currentQueueIndex + 1) % numOnus;    // search from the next queue for a frame to transmit

    for (int i = 0; i < numOnus; i++)
    {
        int idx = (i + startQueueIndex) % numOnus;
        if (queues[idx]->isEmpty() == false)
        {
            deficitCounters[idx] += quanta[idx];
            if (isChannelAvailable[idx] == true)
            {
                int pktLength = (check_and_cast<cPacket *>(queues[currentQueueIndex]->front()))->getByteLength();
                if (deficitCounters[idx] >= pktLength)
                {
                    currentQueueIndex = idx;
                    found = true;
                    return((EtherFrame *)queues[idx]->pop());
                }
            }
        }
    }   // end of for()

    return(NULL);              // return NULL if there is no frame to send
}

//------------------------------------------------------------------------------
//	Event handling functions
//------------------------------------------------------------------------------

///
/// Handles an Ethernet frame from the SNI (i.e., Ethernet switch).
/// Puts the Ethernet frame into a VOQ, if there is enough space; in case
/// there is a tunable transmitter available, schedules the frame for transmission.
/// Otherwise, drops it.
///
/// @param[in] frame	an EtherFrame pointer
///
void OltScheduler2MCDRR::handleEthernetFrameFromSni(EtherFrame *frame)
{
    Enter_Method("handleEthernetFrameFromSni()");

	int ch = frame->getArrivalGate()->getIndex();
    cQueue *queue = queues[ch];
    numQueueReceived[ch]++;

    if (numTxsAvailable > 0)
	{   // transmit the frame immediately
        ASSERT(queue->isEmpty() == true);
        scheduleAt(simTime()+(frame->getBitLength()+INTERFRAME_GAP_BITS)/lineRate, endTxMsg);
        send(frame, "wdmg$o");
        numTxsAvailable--;
        isChannelAvailable[ch] = false;
        currentQueueIndex = ch;
	}
	else
	{   // put the frame into a VOQ
	    bool dropped = enqueue(frame);
	    if (dropped)
	    {
	        numQueueDropped[ch]++;
	    }
	}
}

///
/// Handles the EndTxMsg to model the end of Ethernet frame transmission
/// (including guard band).
///
void OltScheduler2MCDRR::handleEndTxMsg(HybridPonMessage *msg)
{
    ASSERT(numTxsAvailable >= 0);   // check if the number of TXs available is non-negative
    ASSERT(isChannelAvailable[msg->getOnuIdx()] == false);  // check if the channel is not available

    EtherFrame *frame = dequeue();
    if (frame != NULL)
    {
        scheduleAt(simTime() + (frame->getBitLength() + INTERFRAME_GAP_BITS) / lineRate, endTxMsg);
        send(frame, "wdmg$o", currentQueueIndex);
    }
    else
    {
        numTxsAvailable++;
        isChannelAvailable[msg->getOnuIdx()] = true;

        // check if the number of TXs available is not greater than the number of TXs
        ASSERT(numTxsAvailable <= numTransmitters);
    }
}

///
/// Initializes member variables & activities and allocates memories
/// for them, if needed.
///
void OltScheduler2MCDRR::initialize(void)
{
	OltScheduler2::initialize();

	// configuration
	frameCapacity = par("frameCapacity");
	int quantum = par("quantum");   // FIXME: Extend it to a vector later!

	// set subqueues
	queues.assign(numOnus, (cQueue *)NULL);
	for (int i = 0; i < numOnus; i++)
	{
	    queues[i] = new cQueue();
	}

	// state: scheduler
	continued = false;
	currentQueueIndex = 0;
	isChannelAvailable.assign(numOnus, true);
	deficitCounters.assign(numOnus, 0);
	quanta.assign(numOnus, quantum);

	// statistics
    numQueueReceived.assign(numOnus, 0);
    numQueueDropped.assign(numOnus, 0);
}

///
/// Does post processing and deallocates memories manually allocated
/// for member variables.
///
void OltScheduler2MCDRR::finish(void)
{
    unsigned long sumQueueReceived = 0;
    unsigned long sumQueueDropped = 0;
    for (int i=0; i < numOnus; i++)
    {
        std::stringstream ss_received, ss_dropped, ss_shaped, ss_sent;
        ss_received << "packets received by per-VLAN queue[" << i << "]";
        ss_dropped << "packets dropped by per-VLAN queue[" << i << "]";
        recordScalar((ss_received.str()).c_str(), numQueueReceived[i]);
        recordScalar((ss_dropped.str()).c_str(), numQueueDropped[i]);
        sumQueueReceived += numQueueReceived[i];
        sumQueueDropped += numQueueDropped[i];
    }
    recordScalar("overall packet loss rate of per-VLAN queues", sumQueueDropped/double(sumQueueReceived));
}
