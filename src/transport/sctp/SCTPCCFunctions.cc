//
// Copyright (C) 2005-2010 Irene Ruengeler
// Copyright (C) 2009-2010 Thomas Dreibholz
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "SCTPAssociation.h"

#ifdef _MSC_VER
inline double rint(double x) {return floor(x+.5);}
#endif

// #define sctpEV3 std::cout




void SCTPAssociation::recordCwndUpdate(SCTPPathVariables* path)
{
    path->statisticsPathSSthresh->record(path->ssthresh);
    path->statisticsPathCwnd->record(path->cwnd);
}




void SCTPAssociation::initCCParameters(SCTPPathVariables* path)
{
        path->cwnd = (int32)min(4 * path->pmtu, max(2 * path->pmtu, 4380));
    path->ssthresh = state->peerRwnd;
    recordCwndUpdate(path);

    sctpEV3 << simTime() << ":\tCC [initCCParameters]\t" << path->remoteAddress
              << "\tsst=" << path->ssthresh << " cwnd=" << path->cwnd << endl;
}


void SCTPAssociation::cwndUpdateAfterSack()
{
    for (SCTPPathMap::iterator iter = sctpPathMap.begin(); iter != sctpPathMap.end(); iter++) {
        SCTPPathVariables* path = iter->second;
        if(path->fastRecoveryActive == false) {

            // ====== Retransmission required -> reduce congestion window ======
            if(path->requiresRtx) {
                double decreaseFactor = 0.5;

                      sctpEV3 << simTime() << ":\tCC [cwndUpdateAfterSack]\t" << path->remoteAddress
                                 << "\tsst=" << path->ssthresh << " cwnd=" << path->cwnd;

                      path->ssthresh = max((int32)path->cwnd - (int32)rint(decreaseFactor * (double)path->cwnd),
                                                  4 * (int32)path->pmtu);
                      path->cwnd = path->ssthresh;

                      sctpEV3 << "\t=>\tsst=" << path->ssthresh << " cwnd=" << path->cwnd << endl;
                 recordCwndUpdate(path);
                 path->partialBytesAcked = 0;


                 // ====== Fast Recovery ========================================
                 if(state->fastRecoverySupported) {
                    uint32 highestAckOnPath = state->lastTsnAck;
                    for(SCTPQueue::PayloadQueue::iterator pq = retransmissionQ->payloadQueue.begin();
                         pq != retransmissionQ->payloadQueue.end(); pq++) {
                         if( (chunkHasBeenAcked(pq->second) == true) &&
                              (tsnGt(pq->second->tsn, highestAckOnPath)) &&
                              (pq->second->getLastDestinationPath() == path) ) {
                             // T.D. 21.11.09: Only take care of TSNs on the same path!
                             highestAckOnPath = pq->second->tsn;
                         }
                    }
                    /* this can ONLY become TRUE, when Fast Recovery IS supported */
                    path->fastRecoveryActive         = true;
                    path->fastRecoveryExitPoint  = highestAckOnPath;
                    path->fastRecoveryEnteringTime = simTime();

                    sctpEV3 << simTime() << ":\tCC [cwndUpdateAfterSack] Entering Fast Recovery on path "
                              << path->remoteAddress
                              << ", exit point is " << path->fastRecoveryExitPoint << endl;
                }
            }
        }
        else {
            for (SCTPPathMap::iterator iter = sctpPathMap.begin(); iter != sctpPathMap.end(); iter++) {
                SCTPPathVariables* path = iter->second;
                if(path->fastRecoveryActive) {
                    sctpEV3 << simTime() << ":\tCC [cwndUpdateAfterSack] Still in Fast Recovery on path "
                              << path->remoteAddress
                              << ", exit point is " << path->fastRecoveryExitPoint << endl;
                }
            }
        }
    }
}


void SCTPAssociation::updateFastRecoveryStatus(const uint32 lastTsnAck)
{
    for (SCTPPathMap::iterator iter = sctpPathMap.begin(); iter != sctpPathMap.end(); iter++) {
        SCTPPathVariables* path = iter->second;

        if (path->fastRecoveryActive) {
            if ( (tsnGt(lastTsnAck, path->fastRecoveryExitPoint)) ||
                  (lastTsnAck == path->fastRecoveryExitPoint)
            ) {
                path->fastRecoveryActive     = false;
                path->fastRecoveryExitPoint = 0;

                sctpEV3 << simTime() << ":\tCC [cwndUpdateAfterSack] Leaving Fast Recovery on path "
                              << path->remoteAddress
                              << ", lastTsnAck=" << lastTsnAck << endl;
            }
        }
    }
}


void SCTPAssociation::cwndUpdateBytesAcked(SCTPPathVariables* path,
                                                         const uint32         ackedBytes,
                                                         const bool           ctsnaAdvanced)
{
    sctpEV3 << simulation.getSimTime() << "====> cwndUpdateBytesAcked:"
              << " path="                     << path->remoteAddress
              << " ackedBytes="           << ackedBytes
              << " ctsnaAdvanced="        << ((ctsnaAdvanced == true) ? "yes" : "no")
              << " cwnd="                     << path->cwnd
              << " ssthresh="                 << path->ssthresh
              << " ackedBytes="           << ackedBytes
              << " pathOsbBeforeUpdate=" << path->outstandingBytesBeforeUpdate
              << " pathOsb="                  << path->outstandingBytes
              << endl;

    if (path->fastRecoveryActive == false) {
        // T.D. 21.11.09: Increasing cwnd is only allowed when not being in
        //                      Fast Recovery mode!

        // ====== Slow Start ==================================================
        if (path->cwnd <= path->ssthresh)  {
            // ------ Clear PartialBytesAcked counter --------------------------
            path->partialBytesAcked = 0;

            // ------ Increase Congestion Window -------------------------------
            if ((ctsnaAdvanced == true) &&
                 (path->outstandingBytesBeforeUpdate >= path->cwnd)) {
                    sctpEV3 << simTime() << ":\tCC [cwndUpdateBytesAcked-SlowStart]\t" << path->remoteAddress
                              << "\tsst=" << path->ssthresh << " cwnd=" << path->cwnd << " acked=" << ackedBytes;

                    path->cwnd += (int32)min(path->pmtu, ackedBytes);

                    sctpEV3 << "\t=>\tsst=" << path->ssthresh << " cwnd=" << path->cwnd << endl;
                recordCwndUpdate(path);
            }

            // ------ No need to increase Congestion Window --------------------
            else {
                sctpEV3 << simTime() << ":\tCC "
                        << "Not increasing cwnd of path " << path->remoteAddress << " in slow start:\t"
                        << "ctsnaAdvanced="       << ((ctsnaAdvanced == true) ? "yes" : "no") << "\t"
                        << "cwnd="                    << path->cwnd                              << "\t"
                        << "ssthresh="                << path->ssthresh                          << "\t"
                        << "ackedBytes="              << ackedBytes                              << "\t"
                        << "pathOsbBeforeUpdate=" << path->outstandingBytesBeforeUpdate << "\t"
                        << "pathOsb="                 << path->outstandingBytes              << "\t"
                        << "(pathOsbBeforeUpdate >= path->cwnd)="
                        << (path->outstandingBytesBeforeUpdate >= path->cwnd) << endl;
            }
        }

        // ====== Congestion Avoidance ========================================
        else
        {
            // ------ Increase PartialBytesAcked counter -----------------------
            path->partialBytesAcked += ackedBytes;

            double increaseFactor = 1.0;

            // ------ Increase Congestion Window -------------------------------
            if ( (path->partialBytesAcked >= path->cwnd) &&
                  (ctsnaAdvanced == true) &&
                  (path->outstandingBytesBeforeUpdate >= path->cwnd) ) {
                    sctpEV3 << simTime() << ":\tCC [cwndUpdateBytesAcked-CgAvoidance]\t" << path->remoteAddress
                              << "\tsst=" << path->ssthresh << " cwnd=" << path->cwnd << " acked=" << ackedBytes;

                    path->cwnd += (int32)rint(increaseFactor * path->pmtu);

                    sctpEV3 << "\t=>\tsst=" << path->ssthresh << " cwnd=" << path->cwnd << endl;
                recordCwndUpdate(path);
                path->partialBytesAcked =
                    ((path->cwnd < path->partialBytesAcked) ?
                        (path->partialBytesAcked - path->cwnd) : 0);
            }

            // ------ No need to increase Congestion Window -------------------
            else {
                sctpEV3 << simTime() << ":\tCC "
                        << "Not increasing cwnd of path " << path->remoteAddress << " in congestion avoidance: "
                        << "ctsnaAdvanced="       << ((ctsnaAdvanced == true) ? "yes" : "no") << "\t"
                        << "cwnd="                    << path->cwnd                              << "\t"
                        << "ssthresh="                << path->ssthresh                          << "\t"
                        << "ackedBytes="              << ackedBytes                              << "\t"
                        << "pathOsbBeforeUpdate=" << path->outstandingBytesBeforeUpdate << "\t"
                        << "pathOsb="                 << path->outstandingBytes              << "\t"
                        << "(pathOsbBeforeUpdate >= path->cwnd)="
                            << (path->outstandingBytesBeforeUpdate >= path->cwnd)            << "\t"
                        << "partialBytesAcked="   << path->partialBytesAcked                 << "\t"
                        << "(path->partialBytesAcked >= path->cwnd)="
                            << (path->partialBytesAcked >= path->cwnd) << endl;
            }
        }

        // ====== Reset PartialBytesAcked counter if no more outstanding bytes
        if(path->outstandingBytes == 0) {
            path->partialBytesAcked = 0;
        }
    }
    else {
        sctpEV3 << simTime() << ":\tCC "
                << "Not increasing cwnd of path " << path->remoteAddress
                << " during Fast Recovery" << endl;
    }
}


void SCTPAssociation::cwndUpdateAfterRtxTimeout(SCTPPathVariables* path)
{
        sctpEV3 << simTime() << ":\tCC [cwndUpdateAfterRtxTimeout]\t" << path->remoteAddress
                    << "\tsst=" << path->ssthresh << " cwnd=" << path->cwnd;

        path->ssthresh = (int32)max(path->cwnd / 2, 4 * path->pmtu);
        path->cwnd = path->pmtu;

        sctpEV3 << "\t=>\tsst=" << path->ssthresh << " cwnd=" << path->cwnd << endl;
    path->partialBytesAcked         = 0;
    recordCwndUpdate(path);

    // Leave Fast Recovery mode
    if (path->fastRecoveryActive == true) {
        path->fastRecoveryActive     = false;
        path->fastRecoveryExitPoint = 0;
    }
}


void SCTPAssociation::cwndUpdateMaxBurst(SCTPPathVariables* path)
{
        if(path->cwnd > ((path->outstandingBytes + state->maxBurst * path->pmtu))) {
            sctpEV3 << simTime()      << ":\tCC [cwndUpdateMaxBurst]\t" << path->remoteAddress
                    << "\tsst="       << path->ssthresh << " cwnd=" << path->cwnd
                    << "\tosb="       << path->outstandingBytes
                    << "\tmaxBurst=" << state->maxBurst * path->pmtu << endl;

            path->cwnd = path->outstandingBytes + (state->maxBurst * path->pmtu);
            recordCwndUpdate(path);

            sctpEV3 << "\t=>\tsst=" << path->ssthresh << " cwnd=" << path->cwnd << endl;
        }
}


void SCTPAssociation::cwndUpdateAfterCwndTimeout(SCTPPathVariables* path)
{
    // When the association does not transmit data on a given transport address
    // within an RTO, the cwnd of the transport address SHOULD be adjusted to 2*MTU.

        sctpEV3 << simTime() << ":\tCC [cwndUpdateAfterCwndTimeout]\t" << path->remoteAddress
                  << "\tsst=" << path->ssthresh << " cwnd=" << path->cwnd;
        path->cwnd = (int32)min(4 * path->pmtu, max(2 * path->pmtu, 4380));
        sctpEV3 << "\t=>\tsst=" << path->ssthresh << " cwnd=" << path->cwnd << endl;
    recordCwndUpdate(path);
}
