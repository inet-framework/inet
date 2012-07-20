///
/// @file   OltScheduler2MCDRRv3.cc
/// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
/// @date   2012-07-19
///
/// @brief  Implements 'OltScheduler2MCDRRv3' class for a hybrid TDM/WDM-PON OLT.
///
/// @note
/// This file implements an 'OltScheduler2MCDRRv3' class for the 'Multi-Channel
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
// #define DEBUG_OLT_SCHEDULER2MCDRRv3


#include "OltScheduler2.h"

// Register module.
Define_Module(OltScheduler2MCDRRv3);

//------------------------------------------------------------------------------
//	Misc. functions
//------------------------------------------------------------------------------

EtherFrame *OltScheduler2MCDRRv3::dequeue()
{
    int startQueueIndex = (currentQueueIndex + 1) % numOnus;    // search from the next VOQ for a frame to transmit

    for (int i = 0; i < numOnus; i++)
    {
        int idx = (i + startQueueIndex) % numOnus;
        if (queues[idx]->isEmpty() == false)
        {
            if (numPktsScheduled[idx] == 0)
            {
                deficitCounters[idx] += quanta[idx];
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
