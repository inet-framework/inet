// $Id$
//------------------------------------------------------------------------------
//	SchedulerBEDF.cc --
//
//	This file implements an BEDF class for the "Batch Earliest Departure First"
//	scheduling algorithm reported in [1] for SUCCESS WDM-PON, which is based on
//	the 'Batch'	class.
//
//  References:
//  [1] Kyeong Soo Kim, David Gutierrez, Fu-Tai An, and Leonid G. Kazovsky,
//      "Design and performance analysis of scheduling algorithms for WDM-PON
//      under SUCCESS-HPON architecture," IEEE/OSA Journal of Lightwave Technology,
//      vol. 23, no. 11, pp. 3716-3731, Nov. 2005.
//
//	Copyright (C) 2009 Kyeong Soo (Joseph) Kim
//------------------------------------------------------------------------------


// For debugging
// #define DEBUG_SCHEDULER


#include "Scheduler.h"


// Register module.
Define_Module(BEDF);


//------------------------------------------------------------------------------
// BEDF::scheduler --
//
//      Selects the VOQ whose HOL frame has the earliest departure (transmission)
//      time.
//
// Arguments:
//      const simtime_t     TX;                 // earliest available time of given TX
//      const simtime_t     RX;                 // earliest available time of given RX
//      simtime_t           &t;                 // scheduled transmission time
//      int                 &voqsToSchedule;    // # of VOQs to schedule
//      BoolVector          &schedulableVoq;    // vector indicating unempty VOQs
//      Voq                 *voq;               // vector of VOQs
//
// Results:
//      It returns the index of VOQ if the scheduling was successful.
//      Otherwise -1 is returned to indicate that there is no frame to schedule.
//------------------------------------------------------------------------------

int BEDF::scheduler(
    const simtime_t TX,
    const simtime_t RX,
    simtime_t &t,
    int &voqsToSchedule,
    BoolVector &schedulableVoq,
    Voq *voq)
{
    // initialize scheduling variables.
    int numVoqs = 2*numOnus;
    t = simTime() + INF;
    simtime_t tmp = 0;
    int voqIdx = -1;
    HybridPonFrame    *ponFrameToOnu = NULL;

    for (int i=0; i<numVoqs; i++) {
        // Set a starting index to the one next (in a cyclic manner)
        // to the last chosen index for fairness.
        int k = (voqStartIdx + i) % numVoqs;

        if (schedulableVoq[k] == TRUE) {
            if (voq[k].empty() == TRUE) {
                schedulableVoq[k] = FALSE;
                voqsToSchedule--;
            }
            else {
#ifdef DEBUG_SCHEDULER
                ev << "BEDF: batchSchedule: frames waiting in voq [" << k << "]." << endl;
#endif

                ponFrameToOnu = (HybridPonFrame *) (voq[k].front());
                if (ponFrameToOnu->getId() == 0) {
                    // This is a grant frame.
                    // Note that in this case, ch. idx = k - numOnus.
                    int chIdx = k - numOnus;

                    /*tmp = RX + GUARD_TIME - RTT[chIdx];
                    if (TX + GUARD_TIME > tmp) {
                        tmp = TX + GUARD_TIME;
                    }
                    if (CH[chIdx] > tmp) {
                        tmp = CH[chIdx];
                    }*/

					tmp = max(
							max(
								RX + GUARD_TIME - RTT[chIdx] - DS_GRANT_OVERHEAD_SIZE/BITRATE,
								TX + GUARD_TIME
							),
							CH[chIdx] + GUARD_TIME
						);
                }
                else {
                    // This is a normal data frame.

                    /*tmp = TX + GUARD_TIME;
                    if (CH[k] > tmp) {
                        tmp = CH[k];
                    }*/

					tmp = max( TX + GUARD_TIME, CH[k] + GUARD_TIME );
                }
                if (tmp < t) {
                    t = tmp;
                    voqIdx = k;
                }
            }
        }   // end of if()
    }   // end of for()

    voqStartIdx = (voqIdx + 1) % numVoqs;
    return voqIdx;
}
