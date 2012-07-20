///
/// @file   OltScheduler2MCDRRv2.cc
/// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
/// @date   2012-07-19
///
/// @brief  Implements 'OltScheduler2MCDRRv2' class for a hybrid TDM/WDM-PON OLT.
///
/// @note
/// This file implements an 'OltScheduler2MCDRRv2' class for the 'Multi-Channel
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
// #define DEBUG_OLT_SCHEDULER2MCDRRv2


#include "OltScheduler2.h"

// Register module.
Define_Module(OltScheduler2MCDRRv2);

//------------------------------------------------------------------------------
//	Misc. functions
//------------------------------------------------------------------------------

EtherFrame *OltScheduler2MCDRRv2::dequeue()
{
    int startQueueIndex = (currentQueueIndex + 1) % numOnus;    // search from the next VOQ for a frame to transmit

    for (int i = 0; i < numOnus; i++)
    {
        int idx = (i + startQueueIndex) % numOnus;
        if (queues[idx]->isEmpty() == false)
        {
            deficitCounters[idx] += quanta[idx];
            if (numPktsScheduled[idx] == 0)
            {
                currentQueueIndex = idx;
                // we assume that the quantum size is at least as large as the
                // maximum packet size, which means at least one packet is
                // scheduled for transmission as far as the queue is backlogged.
                cQueue::Iterator iter(*queues[idx], 1);
                cPacket *pkt = (cPacket *) iter();
                do
                {
                    deficitCounters[idx] -= pkt->getByteLength();
                    ASSERT(deficitCounters[idx] >= 0);
                    numPktsScheduled[idx]++;
                    iter++;
                    if (iter.end() == true)
                    {
                        break;
                    }
                    else
                    {
                        pkt = (cPacket *) iter();
                    }
                } while (deficitCounters[idx] >= pkt->getByteLength());
                return ((EtherFrame *) queues[idx]->pop());
            }
        }
    } // end of for()

    return(NULL);              // return NULL if there is no frame to send
}

//------------------------------------------------------------------------------
//	Event handling functions
//------------------------------------------------------------------------------

// ///
// /// Handles an Ethernet frame from the SNI (i.e., Ethernet switch).
// /// Puts the Ethernet frame into a VOQ, if there is enough space; in case
// /// there is a tunable transmitter available, schedules the frame for transmission.
// /// Otherwise, drops it.
// ///
// /// @param[in] frame	an EtherFrame pointer
// ///
// void OltScheduler2MCDRRv2::handleEthernetFrameFromSni(EtherFrame *frame)
// {
//     Enter_Method("handleEthernetFrameFromSni()");

//     int ch = frame->getArrivalGate()->getIndex();
//     cQueue *queue = queues[ch];
//     numQueueReceived[ch]++;

//     bool dropped = enqueue(frame);  // enqueue the frame
//     if (dropped)
//     {
//         numQueueDropped[ch]++;
//     }
//     else
//     {
//         if (numTxsAvailable > 0)
//         {   // trigger scheduling
//             EtherFrame *frame = dequeue();
//             if (frame != NULL)
//             {
//                 scheduleAt(simTime() + (frame->getBitLength() + INTERFRAME_GAP_BITS) / lineRate, releaseTxMsg[currentQueueIndex]);
//                 send(frame, "wdmg$o", currentQueueIndex);
//                 isChannelAvailable[currentQueueIndex] = false;
//                 numTxsAvailable--;
//                 ASSERT(numTxsAvailable >= 0);
//                 if (queues[currentQueueIndex]->isEmpty() == true)
//                 {   // reset the deficit counter
//                     deficitCounters[currentQueueIndex] = 0;
//                 }
//             }
//         }
//     }
// }

///
/// Handles the EndTxMsg to model the end of Ethernet frame transmission
/// (including guard band).
///
void OltScheduler2MCDRRv2::handleEndTxMsg(HybridPonMessage *msg)
{
    int ch = msg->getOnuIdx();
    
    ASSERT(numPktsScheduled[ch] > 0);
    numPktsScheduled[ch]--;

    if (numPktsScheduled[ch] > 0)
    {   // there are still packets to transmit as the result of previous scheduling
        EtherFrame *frame = (EtherFrame *) queues[ch]->pop();
        scheduleAt(simTime() + (frame->getBitLength() + INTERFRAME_GAP_BITS) / lineRate, releaseTxMsg[ch]);
        send(frame, "wdmg$o", ch);
        if (queues[ch]->isEmpty() == true)
        {   // reset the deficit counter
            deficitCounters[currentQueueIndex] = 0;
        }
    }
    else
    {   // no more packets to transmit from the current channel; trigger new scheduling
        EtherFrame *frame = dequeue();
        if (frame != NULL)
        {
            scheduleAt(simTime() + (frame->getBitLength() + INTERFRAME_GAP_BITS) / lineRate, releaseTxMsg[currentQueueIndex]);
            send(frame, "wdmg$o", currentQueueIndex);
            if (queues[currentQueueIndex]->isEmpty() == true)
            {   // reset the deficit counter
                deficitCounters[currentQueueIndex] = 0;
            }
        }
        else
        {
            numTxsAvailable++;
            ASSERT(numTxsAvailable <= numTransmitters);
        }
    }
}

///
/// Initializes member variables & activities and allocates memories
/// for them, if needed.
///
void OltScheduler2MCDRRv2::initialize(void)
{
	OltScheduler2MCDRR::initialize();

	// state: scheduler
	numPktsScheduled.assign(numOnus, 0);
}
