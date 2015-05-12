//
// Copyright (C) 2005-2010 Irene Ruengeler
// Copyright (C) 2009-2015 Thomas Dreibholz
// Copyright (C) 2015 Martin Becke
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

#include "inet/transportlayer/sctp/SCTPAssociation.h"

namespace inet {

namespace sctp {

#ifdef _MSC_VER
inline double rint(double x) { return floor(x + .5); }
#endif // ifdef _MSC_VER

// #define sctpEV3 std::cout

#define HIGHSPEED_ENTRIES 73

static inline double GET_SRTT(const double srtt)
{
    return floor(1000.0 * srtt * 8.0);
}

// ====== High-Speed CC cwnd adjustment table from RFC 3649 appendix B ======
struct HighSpeedCwndAdjustmentEntry
{
    int32_t cwndThreshold;
    double increaseFactor;
    double decreaseFactor;
};

static const HighSpeedCwndAdjustmentEntry HighSpeedCwndAdjustmentTable[HIGHSPEED_ENTRIES] = {
    { 38, 1, 0.50 },
    { 118, 2, 0.44 },
    { 221, 3, 0.41 },
    { 347, 4, 0.38 },
    { 495, 5, 0.37 },
    { 663, 6, 0.35 },
    { 851, 7, 0.34 },
    { 1058, 8, 0.33 },
    { 1284, 9, 0.32 },
    { 1529, 10, 0.31 },
    { 1793, 11, 0.30 },
    { 2076, 12, 0.29 },
    { 2378, 13, 0.28 },
    { 2699, 14, 0.28 },
    { 3039, 15, 0.27 },
    { 3399, 16, 0.27 },
    { 3778, 17, 0.26 },
    { 4177, 18, 0.26 },
    { 4596, 19, 0.25 },
    { 5036, 20, 0.25 },
    { 5497, 21, 0.24 },
    { 5979, 22, 0.24 },
    { 6483, 23, 0.23 },
    { 7009, 24, 0.23 },
    { 7558, 25, 0.22 },
    { 8130, 26, 0.22 },
    { 8726, 27, 0.22 },
    { 9346, 28, 0.21 },
    { 9991, 29, 0.21 },
    { 10661, 30, 0.21 },
    { 11358, 31, 0.20 },
    { 12082, 32, 0.20 },
    { 12834, 33, 0.20 },
    { 13614, 34, 0.19 },
    { 14424, 35, 0.19 },
    { 15265, 36, 0.19 },
    { 16137, 37, 0.19 },
    { 17042, 38, 0.18 },
    { 17981, 39, 0.18 },
    { 18955, 40, 0.18 },
    { 19965, 41, 0.17 },
    { 21013, 42, 0.17 },
    { 22101, 43, 0.17 },
    { 23230, 44, 0.17 },
    { 24402, 45, 0.16 },
    { 25618, 46, 0.16 },
    { 26881, 47, 0.16 },
    { 28193, 48, 0.16 },
    { 29557, 49, 0.15 },
    { 30975, 50, 0.15 },
    { 32450, 51, 0.15 },
    { 33986, 52, 0.15 },
    { 35586, 53, 0.14 },
    { 37253, 54, 0.14 },
    { 38992, 55, 0.14 },
    { 40808, 56, 0.14 },
    { 42707, 57, 0.13 },
    { 44694, 58, 0.13 },
    { 46776, 59, 0.13 },
    { 48961, 60, 0.13 },
    { 51258, 61, 0.13 },
    { 53677, 62, 0.12 },
    { 56230, 63, 0.12 },
    { 58932, 64, 0.12 },
    { 61799, 65, 0.12 },
    { 64851, 66, 0.11 },
    { 68113, 67, 0.11 },
    { 71617, 68, 0.11 },
    { 75401, 69, 0.10 },
    { 79517, 70, 0.10 },
    { 84035, 71, 0.10 },
    { 89053, 72, 0.10 },
    { 94717, 73, 0.09 }
};

void SCTPAssociation::updateHighSpeedCCThresholdIdx(SCTPPathVariables *path)
{
    ASSERT(path->highSpeedCCThresholdIdx < HIGHSPEED_ENTRIES);

    if (path->cwnd > HighSpeedCwndAdjustmentTable[path->highSpeedCCThresholdIdx].cwndThreshold * path->pmtu) {
        while ((path->cwnd > HighSpeedCwndAdjustmentTable[path->highSpeedCCThresholdIdx].cwndThreshold * path->pmtu) &&
                (path->highSpeedCCThresholdIdx < HIGHSPEED_ENTRIES)) {
            path->highSpeedCCThresholdIdx++;
        }
    } else {
        while ((path->cwnd <= HighSpeedCwndAdjustmentTable[path->highSpeedCCThresholdIdx].cwndThreshold * path->pmtu) &&
                (path->highSpeedCCThresholdIdx > 0)) {
            path->highSpeedCCThresholdIdx--;
        }
    }
}

void SCTPAssociation::cwndUpdateBeforeSack()
{
    // First, calculate per-path values.
    for (auto otherPathIterator = sctpPathMap.begin();
         otherPathIterator != sctpPathMap.end(); otherPathIterator++)
    {
        SCTPPathVariables *otherPath = otherPathIterator->second;
        otherPath->utilizedCwnd = otherPath->outstandingBytesBeforeUpdate;
    }

    // Calculate per-path-group values.
    for (auto currentPathIterator = sctpPathMap.begin();
         currentPathIterator != sctpPathMap.end(); currentPathIterator++)
    {
        SCTPPathVariables *currentPath = currentPathIterator->second;

        currentPath->cmtGroupPaths = 0;
        currentPath->cmtGroupTotalCwnd = 0;
        currentPath->cmtGroupTotalSsthresh = 0;
        currentPath->cmtGroupTotalUtilizedCwnd = 0;
        currentPath->cmtGroupTotalCwndBandwidth = 0.0;
        currentPath->cmtGroupTotalUtilizedCwndBandwidth = 0.0;

        double qNumerator = 0.0;
        double qDenominator = 0.0;
        for (SCTPPathMap::const_iterator otherPathIterator = sctpPathMap.begin();
             otherPathIterator != sctpPathMap.end(); otherPathIterator++)
        {
            const SCTPPathVariables *otherPath = otherPathIterator->second;
            if (otherPath->cmtCCGroup == currentPath->cmtCCGroup) {
                currentPath->cmtGroupPaths++;

                currentPath->cmtGroupTotalCwnd += otherPath->cwnd;
                currentPath->cmtGroupTotalSsthresh += otherPath->ssthresh;
                currentPath->cmtGroupTotalCwndBandwidth += otherPath->cwnd / GET_SRTT(otherPath->srtt.dbl());

                if ((otherPath->blockingTimeout < 0.0) || (otherPath->blockingTimeout < simTime())) {
                    currentPath->cmtGroupTotalUtilizedCwnd += otherPath->utilizedCwnd;
                    currentPath->cmtGroupTotalUtilizedCwndBandwidth += otherPath->utilizedCwnd / GET_SRTT(otherPath->srtt.dbl());
                }

                qNumerator = max(qNumerator, otherPath->cwnd / (pow(GET_SRTT(otherPath->srtt.dbl()), 2.0)));
                qDenominator = qDenominator + (otherPath->cwnd / GET_SRTT(otherPath->srtt.dbl()));
            }
        }
        currentPath->cmtGroupAlpha = currentPath->cmtGroupTotalCwnd * (qNumerator / pow(qDenominator, 2.0));
    }
}

static uint32 updateMPTCP(const uint32 w,
        const uint32 totalW,
        double a,
        const uint32 mtu,
        const uint32 ackedBytes)
{
    const uint32 increase = max(1,
                min((uint32)ceil((double)w * a * (double)min(ackedBytes, mtu) / (double)totalW),
                        (uint32)min(ackedBytes, mtu)));
    return w + increase;
}

void SCTPAssociation::recalculateOLIABasis() {
    // it is necessary to calculate all flow information
    double assoc_best_paths_l_rXl_r__rtt_r = 0.0;
    //max_w_paths: The set of paths in all_paths with largest congestion windows.
    //https://tools.ietf.org/html/draft-khalili-mptcp-congestion-control-05
    uint32 max_w_paths = 0;
    uint32 max_w_paths_cnt = 0; (void)max_w_paths_cnt; // FIXME this variable is unused
    uint32 best_paths_cnt = 0;

    // Create the sets
    int cnt = 0;
    assocCollectedPaths.clear();
    assocBestPaths.clear();
    assocMaxWndPaths.clear();
    for (SCTPPathMap::iterator iter = sctpPathMap.begin();
            iter != sctpPathMap.end(); iter++, cnt++) {
        SCTPPathVariables* path = iter->second;
        bool next = false;
        double r_sRTT = GET_SRTT(path->srtt.dbl());
        double r_l_rXl_r__rtt_r = ((path->oliaSentBytes
                * path->oliaSentBytes) / r_sRTT);
        if (assocBestPaths.empty()) {
            assoc_best_paths_l_rXl_r__rtt_r = r_l_rXl_r__rtt_r;
            assocBestPaths.insert(std::make_pair(cnt, path));
            next = true;
        }
        if (assocMaxWndPaths.empty()) {
            max_w_paths = path->cwnd;
            assocMaxWndPaths.insert(std::make_pair(cnt, path));
            next = true;
        }
        if (next)
            continue;
        // set up the sets
        if (r_l_rXl_r__rtt_r > assoc_best_paths_l_rXl_r__rtt_r) {
            assoc_best_paths_l_rXl_r__rtt_r = r_l_rXl_r__rtt_r;
            assocBestPaths.insert(std::make_pair(cnt, path));
            assocBestPaths.erase(best_paths_cnt);

            best_paths_cnt = cnt;
            next = true;
        }
        if (path->cwnd > max_w_paths) {
            max_w_paths = path->cwnd;
            assocMaxWndPaths.insert(std::make_pair(cnt, path));
            assocMaxWndPaths.erase(best_paths_cnt);
            max_w_paths_cnt = cnt;
            next = true;
        }
        if (next)
            continue;

        assocCollectedPaths.insert(std::make_pair(cnt, path));

    }
}

uint32 SCTPAssociation::updateOLIA(uint32 w, const uint32 s,
        const uint32 totalW, double a, const uint32 mtu,
        const uint32 ackedBytes, SCTPPathVariables* path) {
    int32 increase = 0;
    bool isInCollectedPath = false;
    bool isMaxWndPaths = false;

    if ((!(w < s)) && (!path->fastRecoveryActive)) {
        // in CA
        recalculateOLIABasis();

        int cnt = 0;
        for (SCTPPathCollection::iterator it = assocCollectedPaths.begin();
                it != assocCollectedPaths.end(); it++, cnt++) {
            if (it->second == path) {
                isInCollectedPath = true;
                break;
            }
        }
        cnt = 0;
        for (SCTPPathCollection::iterator it = assocMaxWndPaths.begin();
                it != assocMaxWndPaths.end(); it++, cnt++) {
            if (it->second == path) {
                isMaxWndPaths = true;
                break;
            }
        }

        double r_sRTT = GET_SRTT(path->srtt.dbl());

        double numerator1 = path->cwnd / (r_sRTT * r_sRTT);
        double denominator1 = 0;

        for (SCTPPathMap::iterator iter = sctpPathMap.begin();
                iter != sctpPathMap.end(); iter++) {
            SCTPPathVariables* p_path = iter->second;
            double p_sRTT = GET_SRTT(p_path->srtt.dbl());
            denominator1 += (p_path->cwnd / p_sRTT);
        }
        denominator1 = denominator1 * denominator1;
        double term1 = numerator1 / denominator1;

        if (isInCollectedPath) {
            /*
             For each ACK on the path r:
             - If r is in collected_paths, increase w_r by

             w_r/rtt_r^2                          1
             -------------------    +     -----------------------       (2)
             (SUM (w_p/rtt_p))^2    w_r * number_of_paths * |collected_paths|

             multiplied by MSS_r * bytes_acked.
             */

            double numerator2 = 1 / sctpPathMap.size();
            double denominator2 = assocCollectedPaths.size();
            double term2 = 0.0;
            if (denominator2 > 0.0) {
                term2 = numerator2 / denominator2;
            }
            increase = (uint32) ceil(
                    (term1 * path->cwnd * path->pmtu) + (term2 * path->pmtu));
        } else if ((isMaxWndPaths) && (!assocCollectedPaths.empty())) {
            /*
             - If r is in max_w_paths and if collected_paths is not empty,
             increase w_r by

             w_r/rtt_r^2                         1
             --------------------    -     ------------------------     (3)
             (SUM (w_p/rtt_p))^2     w_r * number_of_paths * |max_w_paths|

             multiplied by MSS_r * bytes_acked.
             */
            double numerator2 = 1.0 / (double)sctpPathMap.size();
            double denominator2 = assocMaxWndPaths.size();
            double term2 = 0.0;
            if (denominator2 > 0.0) {
                term2 = numerator2 / denominator2;
            }
            increase = (int32) ceil(
                    (term1 * path->cwnd * path->pmtu) - (term2 * path->pmtu)); // TODO
        } else {
            /*
             - Otherwise, increase w_r by

             (w_r/rtt_r^2)
             ----------------------------------           (4)
             (SUM (w_p/rtt_p))^2

             multiplied by MSS_r * bytes_acked.
             */
            increase = (int32) ceil(term1 * path->cwnd * path->pmtu); // TODO std::min(acked,
        }
    } else {
        increase = (int32) min(path->pmtu, ackedBytes);  // slow start
    }
    return (w + increase);
}

void SCTPAssociation::recordCwndUpdate(SCTPPathVariables* path)
{
    if (path == nullptr) {
        uint32 totalSsthresh = 0.0;
        uint32 totalCwnd = 0.0;
        double totalBandwidth = 0.0;
        for (auto pathIterator = sctpPathMap.begin();
             pathIterator != sctpPathMap.end(); pathIterator++)
        {
            SCTPPathVariables *path = pathIterator->second;
            totalSsthresh += path->ssthresh;
            totalCwnd += path->cwnd;
            totalBandwidth += path->cwnd / GET_SRTT(path->srtt.dbl());
        }
        statisticsTotalSSthresh->record(totalSsthresh);
        statisticsTotalCwnd->record(totalCwnd);
        statisticsTotalBandwidth->record(totalBandwidth);
    }
    else {
        path->statisticsPathSSthresh->record(path->ssthresh);
        path->statisticsPathCwnd->record(path->cwnd);
        path->statisticsPathBandwidth->record(path->cwnd / GET_SRTT(path->srtt.dbl()));
    }
}

uint32 SCTPAssociation::getInitialCwnd(const SCTPPathVariables *path) const
{
    uint32 newCwnd;

    const uint32 upperLimit = (state->initialWindow > 0) ? (state->initialWindow * path->pmtu) : max(2 * path->pmtu, 4380);
    if ((state->allowCMT == false) || (state->cmtCCVariant == SCTPStateVariables::CCCV_CMT)) {
        newCwnd = (int32)min((state->initialWindow > 0) ? (state->initialWindow * path->pmtu) : (4 * path->pmtu),
                    upperLimit);
    }
    else {
        newCwnd = (int32)min((int32)ceil(((state->initialWindow > 0) ?
                                          (state->initialWindow * path->pmtu) :
                                          (4 * path->pmtu)) / (double)sctpPathMap.size()),
                    upperLimit);
        if (newCwnd < path->pmtu) {    // T.D. 09.09.2010: cwnd < MTU makes no sense ...
            newCwnd = path->pmtu;
        }
    }    return newCwnd;
}

void SCTPAssociation::initCCParameters(SCTPPathVariables *path)
{
    path->cwnd = getInitialCwnd(path);
    path->ssthresh = state->peerRwnd;
    recordCwndUpdate(path);

    EV_DEBUG << simTime() << ":\tCC [initCCParameters]\t" << path->remoteAddress
             << " (cmtCCGroup=" << path->cmtCCGroup << ")"
             << "\tsst=" << path->ssthresh << " cwnd=" << path->cwnd << endl;
    assocBestPaths.clear();
    assocMaxWndPaths.clear();
}

int32 SCTPAssociation::rpPathBlockingControl(SCTPPathVariables *path, const double reduction)
{
    // ====== Compute new cwnd ===============================================
    const int32 newCwnd = (int32)ceil(path->cwnd - reduction);
    // NOTE: newCwnd may be negative!
    // ====== Block path if newCwnd < 1 MTU ==================================
    if ((state->rpPathBlocking == true) && (newCwnd < (int32)path->pmtu)) {
        if ((path->blockingTimeout < 0.0) || (path->blockingTimeout < simTime())) {
            // printf("a=%1.9f b=%1.9f   a=%d b=%d\n", path->blockingTimeout.dbl(), simTime().dbl(), (path->blockingTimeout < 0.0), (path->blockingTimeout < simTime()) );

            const simtime_t timeout = (state->rpScaleBlockingTimeout == true) ?
                path->cmtGroupPaths * path->pathRto :
                path->pathRto;
            EV << "Blocking " << path->remoteAddress << " for " << timeout << endl;

            path->blockingTimeout = simTime() + timeout;
            assert(!path->BlockingTimer->isScheduled());
            startTimer(path->BlockingTimer, timeout);
        }
    }
    return newCwnd;
}

void SCTPAssociation::cwndUpdateAfterSack()
{
    for (auto iter = sctpPathMap.begin(); iter != sctpPathMap.end(); iter++) {
        SCTPPathVariables *path = iter->second;
        if (path->fastRecoveryActive == false) {
            // ====== Retransmission required -> reduce congestion window ======
            if (path->requiresRtx) {
                double decreaseFactor = 0.5;
                EV << assocId << ": " << simTime() << ":\tCC [cwndUpdateAfterSack]\t" << path->remoteAddress
                   << " (cmtCCGroup=" << path->cmtCCGroup << ")"
                   << "\tsst=" << path->ssthresh
                   << "\tcwnd=" << path->cwnd
                   << "\tSST=" << path->cmtGroupTotalSsthresh
                   << "\tCWND=" << path->cmtGroupTotalCwnd
                   << "\tBW.CWND=" << path->cmtGroupTotalCwndBandwidth;
                if (state->highSpeedCC == true) {
                    decreaseFactor = HighSpeedCwndAdjustmentTable[path->highSpeedCCThresholdIdx].decreaseFactor;
                    EV << "\tHighSpeedDecreaseFactor=" << decreaseFactor;
                }

                // ====== SCTP or CMT-SCTP (independent congestion control) =====
                if ((state->allowCMT == false) ||
                    (state->cmtCCVariant == SCTPStateVariables::CCCV_CMT))
                {
                    EV_INFO << simTime() << ":\tCC [cwndUpdateAfterSack]\t" << path->remoteAddress
                            << "\tsst=" << path->ssthresh << " cwnd=" << path->cwnd;

                    path->ssthresh = max((int32)path->cwnd - (int32)rint(decreaseFactor * (double)path->cwnd),
                                4 * (int32)path->pmtu);
                    path->cwnd = path->ssthresh;
                }
                // ====== Resource Pooling ======================================
                else {
                    // ====== CMT/RP-SCTPv1 Fast Retransmit ======================
                    if (state->cmtCCVariant == SCTPStateVariables::CCCV_CMTRPv1) {
                        const double sstRatio = (double)path->ssthresh / (double)path->cmtGroupTotalSsthresh;
                        const int32 reducedCwnd = rpPathBlockingControl(path, rint(path->cmtGroupTotalCwnd * decreaseFactor));
                        path->ssthresh = max(reducedCwnd,
                                    max((int32)path->pmtu,
                                            (int32)ceil((double)state->rpMinCwnd * (double)path->pmtu * sstRatio)));
                        path->cwnd = path->ssthresh;
                    }
                    // ====== CMT/RPv2-SCTP Fast Retransmit ======================
                    else if (state->cmtCCVariant == SCTPStateVariables::CCCV_CMTRPv2) {
                        // Bandwidth is based on cwnd, *not* ssthresh!
                        const double pathBandwidth = path->cwnd / GET_SRTT(path->srtt.dbl());
                        const double bandwidthToGive = path->cmtGroupTotalCwndBandwidth / 2.0;
                        const double reductionFactor = max(0.5, bandwidthToGive / pathBandwidth);
                        const int32 reducedCwnd = rpPathBlockingControl(path, reductionFactor * path->cwnd);
                        path->ssthresh = (int32)max(reducedCwnd, (int32)state->rpMinCwnd * (int32)path->pmtu);
                        path->cwnd = path->ssthresh;
                    }
                    // ====== Like MPTCP Fast Retransmit =========================
                    else if(state->cmtCCVariant == SCTPStateVariables::CCCV_CMT_LIA) {
                        // Just like plain CMT-SCTP ...
                        const int32 reducedCwnd = rpPathBlockingControl(path, rint(decreaseFactor * (double)path->cwnd));
                        path->ssthresh = max(reducedCwnd, (int32)state->rpMinCwnd * (int32)path->pmtu);
                        path->cwnd = path->ssthresh;
                    }
                    else if (state->cmtCCVariant == SCTPStateVariables::CCCV_CMT_OLIA) {
                        // like draft
                        path->ssthresh = max((int32) path->cwnd-(int32) rint(decreaseFactor * (double) path->cwnd),
                                4 * (int32) path->pmtu);
                        path->cwnd = path->ssthresh;
                    }
                    // ====== TEST Fast Retransmit ===============================
                    else if (state->cmtCCVariant == SCTPStateVariables::CCCV_CMTRP_Test1) {
                        // Bandwidth is based on cwnd, *not* ssthresh!
                        const double pathBandwidth = path->cwnd / GET_SRTT(path->srtt.dbl());
                        const double bandwidthToGive = path->cmtGroupTotalCwndBandwidth / 2.0;
                        const double reductionFactor = max(0.5, bandwidthToGive / pathBandwidth);
                        const int32 reducedCwnd = rpPathBlockingControl(path, reductionFactor * path->cwnd);
                        path->ssthresh = (int32)max(reducedCwnd, (int32)state->rpMinCwnd * (int32)path->pmtu);
                        path->cwnd = path->ssthresh;
                    }
                    // ====== TEST Fast Retransmit ===============================
                    else if (state->cmtCCVariant == SCTPStateVariables::CCCV_CMTRP_Test2) {
                        // Just like CMT-SCTP ...
                        const int32 reducedCwnd = rpPathBlockingControl(path, rint(decreaseFactor * (double)path->cwnd));
                        path->ssthresh = max(reducedCwnd, (int32)state->rpMinCwnd * (int32)path->pmtu);
                        path->cwnd = path->ssthresh;
                    }
                    // ====== Other -> error =====================================
                    else {
                        throw cRuntimeError("Implementation for this cmtCCVariant is missing!");
                    }
                }

                EV_INFO << "\t=>\tsst=" << path->ssthresh << " cwnd=" << path->cwnd << endl;
                recordCwndUpdate(path);
                path->partialBytesAcked = 0;
                path->vectorPathPbAcked->record(path->partialBytesAcked);
                if (state->highSpeedCC == true) {
                    updateHighSpeedCCThresholdIdx(path);
                }

                // ====== Fast Recovery ========================================
                if (state->fastRecoverySupported) {
                    uint32 highestAckOnPath = state->lastTsnAck;
                    uint32 highestOutstanding = state->lastTsnAck;
                    for (SCTPQueue::PayloadQueue::const_iterator chunkIterator = retransmissionQ->payloadQueue.begin();
                         chunkIterator != retransmissionQ->payloadQueue.end(); chunkIterator++)
                    {
                        const SCTPDataVariables *chunk = chunkIterator->second;
                        if (chunk->getLastDestinationPath() == path) {
                            if (chunkHasBeenAcked(chunk)) {
                                if (tsnGt(chunk->tsn, highestAckOnPath)) {
                                    highestAckOnPath = chunk->tsn;
                                }
                            }
                            else {
                                if (tsnGt(chunk->tsn, highestOutstanding)) {
                                    highestOutstanding = chunk->tsn;
                                }
                            }
                        }
                    }
                    path->oliaSentBytes = 0;
                    /* this can ONLY become TRUE, when Fast Recovery IS supported */
                    path->fastRecoveryActive = true;
                    path->fastRecoveryExitPoint = highestOutstanding;
                    path->fastRecoveryEnteringTime = simTime();
                    path->vectorPathFastRecoveryState->record(path->cwnd);

                    EV_INFO << simTime() << ":\tCC [cwndUpdateAfterSack] Entering Fast Recovery on path "
                            << path->remoteAddress
                            << ", exit point is " << path->fastRecoveryExitPoint
                            << ", pseudoCumAck=" << path->pseudoCumAck
                            << ", rtxPseudoCumAck=" << path->rtxPseudoCumAck << endl;
                }
            }
        }
        else {
            for (auto iter = sctpPathMap.begin(); iter != sctpPathMap.end(); iter++) {
                SCTPPathVariables *path = iter->second;
                if (path->fastRecoveryActive) {
                    EV_INFO << assocId << ": " << simTime() << ":\tCC [cwndUpdateAfterSack] Still in Fast Recovery on path "
                            << path->remoteAddress
                            << ", exit point is " << path->fastRecoveryExitPoint << endl;
                }
            }
        }
    }
}

void SCTPAssociation::updateFastRecoveryStatus(const uint32 lastTsnAck)
{
    for (auto iter = sctpPathMap.begin(); iter != sctpPathMap.end(); iter++) {
        SCTPPathVariables *path = iter->second;

        if (path->fastRecoveryActive) {
            if ((tsnGt(lastTsnAck, path->fastRecoveryExitPoint)) ||
                (lastTsnAck == path->fastRecoveryExitPoint) || ((state->allowCMT) && (state->cmtUseFRC) &&
                                                                ((path->newPseudoCumAck && tsnGt(path->pseudoCumAck, path->fastRecoveryExitPoint)) ||
                                                                 (path->newRTXPseudoCumAck && tsnGt(path->rtxPseudoCumAck, path->fastRecoveryExitPoint)))))
            {
                path->fastRecoveryActive = false;
                path->fastRecoveryExitPoint = 0;

                EV_INFO << simTime() << ":\tCC [cwndUpdateAfterSack] Leaving Fast Recovery on path "
                        << path->remoteAddress
                        << ", lastTsnAck=" << lastTsnAck
                        << ", pseudoCumAck=" << path->pseudoCumAck
                        << ", rtxPseudoCumAck=" << path->rtxPseudoCumAck
                        << ", newPseudoCumAck=" << path->newPseudoCumAck
                        << ", newRTXPseudoCumAck=" << path->newRTXPseudoCumAck
                        << endl;
            }
        }
    }
}

void SCTPAssociation::cwndUpdateBytesAcked(SCTPPathVariables *path,
        const uint32 ackedBytes,
        const bool ctsnaAdvanced)
{
    EV_INFO << simTime() << "====> cwndUpdateBytesAcked:"
            << " path=" << path->remoteAddress
            << " ackedBytes=" << ackedBytes
            << " ctsnaAdvanced=" << ((ctsnaAdvanced == true) ? "yes" : "no")
            << " cwnd=" << path->cwnd
            << " ssthresh=" << path->ssthresh
            << " ackedBytes=" << ackedBytes
            << " pathOsbBeforeUpdate=" << path->outstandingBytesBeforeUpdate
            << " pathOsb=" << path->outstandingBytes
            << endl;

    if (path->fastRecoveryActive == false) {
        // T.D. 21.11.09: Increasing cwnd is only allowed when not being in
        //                Fast Recovery mode!

        // ====== Slow Start ==================================================
        if (path->cwnd <= path->ssthresh) {
            // ------ Clear PartialBytesAcked counter --------------------------
            path->partialBytesAcked = 0;

            // ------ Increase Congestion Window -------------------------------
            if ((ctsnaAdvanced == true) &&
                ((path->outstandingBytesBeforeUpdate >= path->cwnd) ||
                 ((state->strictCwndBooking) && (path->outstandingBytesBeforeUpdate + path->pmtu > path->cwnd))))
            {
                EV_INFO << assocId << ": " << simTime() << ":\tCC [cwndUpdateBytesAcked-SlowStart]\t" << path->remoteAddress
                        << " (cmtCCGroup=" << path->cmtCCGroup << ")"
                        << "\tacked=" << ackedBytes
                        << "\tsst=" << path->ssthresh
                        << "\tcwnd=" << path->cwnd
                        << "\tSST=" << path->cmtGroupTotalSsthresh
                        << "\tCWND=" << path->cmtGroupTotalCwnd
                        << "\tBW.CWND=" << path->cmtGroupTotalCwndBandwidth;

                // ====== SCTP or CMT-SCTP (independent congestion control) =====
                if ((state->allowCMT == false) || (state->cmtCCVariant == SCTPStateVariables::CCCV_CMT)) {
                    path->cwnd += (int32)min(path->pmtu, ackedBytes);
                }
                // ====== Resource Pooling Slow Start ===========================
                else {
                    // ====== CMT/RPv1-SCTP Slow Start ===========================
                    if (state->cmtCCVariant == SCTPStateVariables::CCCV_CMTRPv1) {
                        const double sstRatio = (double)path->ssthresh / (double)path->cmtGroupTotalSsthresh;
                        path->cwnd += (int32)ceil(min(path->pmtu, ackedBytes) * sstRatio);
                    }
                    // ====== CMT/RPv2-SCTP Slow Start ===========================
                    else if (state->cmtCCVariant == SCTPStateVariables::CCCV_CMTRPv2) {
                        // Increase ratio based on cwnd bandwidth share!
                        const double increaseRatio = ((double)path->cwnd / GET_SRTT(path->srtt.dbl()))
                            / (double)path->cmtGroupTotalCwndBandwidth;
                        path->cwnd += (int32)ceil(min(path->pmtu, ackedBytes) * increaseRatio);
                    }
                    // ====== Like MPTCP Slow Start ==============================
                    else if(state->cmtCCVariant == SCTPStateVariables::CCCV_CMT_LIA) {
                        // T.D. 14.08.2011: Rewrote MPTCP-Like CC code
                        path->cwnd = updateMPTCP(path->cwnd, path->cmtGroupTotalCwnd,
                                                 path->cmtGroupAlpha, path->pmtu, ackedBytes);
                    }
                    else if (state->cmtCCVariant == SCTPStateVariables::CCCV_CMT_OLIA) {
                        // OLIA see draft
                        path->cwnd = updateOLIA(path->cwnd, path->ssthresh,
                                path->cmtGroupTotalCwnd, path->cmtGroupAlpha,
                                path->pmtu, ackedBytes, path);
                    }
                    // ====== TEST Slow Start ====================================
                    else if (state->cmtCCVariant == SCTPStateVariables::CCCV_CMTRP_Test1) {
                        // Increase ratio based on cwnd bandwidth share!
                        const double increaseRatio = ((double)path->utilizedCwnd / GET_SRTT(path->srtt.dbl()))
                            / (double)path->cmtGroupTotalUtilizedCwndBandwidth;
                        path->cwnd += (int32)ceil(min(path->pmtu, ackedBytes) * increaseRatio);
                    }
                    // ====== Like MPTCP Slow Start ==============================
                    else if (state->cmtCCVariant == SCTPStateVariables::CCCV_CMTRP_Test2) {
                        path->cwnd = updateMPTCP(path->cwnd, path->cmtGroupTotalCwnd,
                                    path->cmtGroupAlpha, path->pmtu, ackedBytes);
                    }
                    // ====== Other -> error =====================================
                    else {
                        throw cRuntimeError("Implementation for this cmtCCVariant is missing!");
                    }
                }
                path->vectorPathPbAcked->record(path->partialBytesAcked);
                EV << "\t=>\tsst=" << path->ssthresh
                   << "\tcwnd=" << path->cwnd << endl;

                recordCwndUpdate(path);
            }
            // ------ No need to increase Congestion Window --------------------
            else {
                EV_INFO << assocId << ": " << simTime() << ":\tCC "
                        << "Not increasing cwnd of path " << path->remoteAddress << " in slow start:\t"
                        << "ctsnaAdvanced=" << ((ctsnaAdvanced == true) ? "yes" : "no") << "\t"
                        << "cwnd=" << path->cwnd << "\t"
                        << "ssthresh=" << path->ssthresh << "\t"
                        << "ackedBytes=" << ackedBytes << "\t"
                        << "pathOsbBeforeUpdate=" << path->outstandingBytesBeforeUpdate << "\t"
                        << "pathOsb=" << path->outstandingBytes << "\t"
                        << "(pathOsbBeforeUpdate >= path->cwnd)="
                        << (path->outstandingBytesBeforeUpdate >= path->cwnd) << endl;
            }
        }
        // ====== Congestion Avoidance ========================================
        else {
            // ------ Increase PartialBytesAcked counter -----------------------
            path->partialBytesAcked += ackedBytes;

            // ------ Increase Congestion Window -------------------------------
            double increaseFactor = 1.0;
            if (state->highSpeedCC == true) {
                updateHighSpeedCCThresholdIdx(path);
                increaseFactor = HighSpeedCwndAdjustmentTable[path->highSpeedCCThresholdIdx].increaseFactor;
                EV << "HighSpeedCC Increase: factor=" << increaseFactor << endl;
            }

            const bool avancedAndEnoughOutstanding =
                (ctsnaAdvanced == true) &&
                ((path->outstandingBytesBeforeUpdate >= path->cwnd) ||
                 ((state->strictCwndBooking) &&
                  (path->outstandingBytesBeforeUpdate + path->pmtu > path->cwnd)));
            const bool enoughPartiallyAcked =
                (path->partialBytesAcked >= path->cwnd) ||
                ((state->strictCwndBooking) &&
                 (path->partialBytesAcked >= path->pmtu) &&
                 (path->partialBytesAcked + path->pmtu > path->cwnd));

            if (avancedAndEnoughOutstanding && enoughPartiallyAcked) {
                EV << assocId << ": " << simTime() << ":\tCC [cwndUpdateBytesAcked-CgAvoidance]\t" << path->remoteAddress
                   << " (cmtCCGroup=" << path->cmtCCGroup << ")"
                   << "\tacked=" << ackedBytes
                   << "\tsst=" << path->ssthresh
                   << "\tcwnd=" << path->cwnd
                   << "\tSST=" << path->cmtGroupTotalSsthresh
                   << "\tCWND=" << path->cmtGroupTotalCwnd
                   << "\tBW.CWND=" << path->cmtGroupTotalCwndBandwidth;

                // ====== SCTP or CMT-SCTP (independent congestion control) =====
                if ((state->allowCMT == false) || (state->cmtCCVariant == SCTPStateVariables::CCCV_CMT)) {
                    path->cwnd += (int32)rint(increaseFactor * path->pmtu);
                }
                // ====== Resource Pooling Congestion Avoidance =================
                else {
                    // ====== CMT/RP-SCTP Congestion Avoidance ===================
                    if (state->cmtCCVariant == SCTPStateVariables::CCCV_CMTRPv1) {
                        const double sstRatio = (double)path->ssthresh / (double)path->cmtGroupTotalSsthresh;
                        path->cwnd += (int32)ceil(increaseFactor * path->pmtu * sstRatio);
                    }
                    // ====== CMT/RPv2-SCTP Congestion Avoidance =================
                    else if (state->cmtCCVariant == SCTPStateVariables::CCCV_CMTRPv2) {
                        // Increase ratio based on cwnd bandwidth share!
                        const double increaseRatio = ((double)path->cwnd / GET_SRTT(path->srtt.dbl()))
                            / (double)path->cmtGroupTotalCwndBandwidth;
                        path->cwnd += (int32)ceil(increaseFactor * path->pmtu * increaseRatio);
                    }
                    // ====== Like MPTCP Congestion Avoidance ====================
                    else if(state->cmtCCVariant == SCTPStateVariables::CCCV_CMT_LIA) {
                        // T.D. 14.08.2011: Rewrote MPTCP-Like CC code
                        path->cwnd = updateMPTCP(path->cwnd, path->cmtGroupTotalCwnd,
                                    path->cmtGroupAlpha, path->pmtu, path->pmtu);
                    }
                    else if (state->cmtCCVariant == SCTPStateVariables::CCCV_CMT_OLIA) {
                        // like draft
                        path->cwnd = updateOLIA(path->cwnd, path->ssthresh,
                                path->cmtGroupTotalCwnd, path->cmtGroupAlpha,
                                path->pmtu, path->pmtu, path);
                    }
                    // ====== TEST Congestion Avoidance ==========================
                    else if (state->cmtCCVariant == SCTPStateVariables::CCCV_CMTRP_Test1) {
                        // Increase ratio based on cwnd bandwidth share!
                        const double increaseRatio = ((double)path->utilizedCwnd / GET_SRTT(path->srtt.dbl()))
                            / (double)path->cmtGroupTotalUtilizedCwndBandwidth;
                        path->cwnd += (int32)ceil(increaseFactor * path->pmtu * increaseRatio);
                    }
                    // ====== TEST Congestion Avoidance ==========================
                    else if (state->cmtCCVariant == SCTPStateVariables::CCCV_CMTRP_Test2) {
                        path->cwnd = updateMPTCP(path->cwnd, path->cmtGroupTotalCwnd,
                                    path->cmtGroupAlpha, path->pmtu, path->pmtu);
                    }
                    // ====== Other -> error =====================================
                    else {
                        throw cRuntimeError("Implementation for this cmtCCVariant is missing!");
                    }
                }
                EV << "\t=>\tsst=" << path->ssthresh
                   << "\tcwnd=" << path->cwnd << endl;

                recordCwndUpdate(path);
                path->partialBytesAcked =
                    ((path->cwnd < path->partialBytesAcked) ?
                     (path->partialBytesAcked - path->cwnd) : 0);
            }
            // ------ No need to increase Congestion Window -------------------
            else {
                EV_INFO << assocId << ": " << simTime() << ":\tCC "
                        << "Not increasing cwnd of path " << path->remoteAddress << " in congestion avoidance: "
                        << "ctsnaAdvanced=" << ((ctsnaAdvanced == true) ? "yes" : "no") << "\t"
                        << "cwnd=" << path->cwnd << "\t"
                        << "ssthresh=" << path->ssthresh << "\t"
                        << "ackedBytes=" << ackedBytes << "\t"
                        << "pathOsbBeforeUpdate=" << path->outstandingBytesBeforeUpdate << "\t"
                        << "pathOsb=" << path->outstandingBytes << "\t"
                        << "(pathOsbBeforeUpdate >= path->cwnd)="
                        << (path->outstandingBytesBeforeUpdate >= path->cwnd) << "\t"
                        << "partialBytesAcked=" << path->partialBytesAcked << "\t"
                        << "(path->partialBytesAcked >= path->cwnd)="
                        << (path->partialBytesAcked >= path->cwnd) << endl;
            }
        }

        // ====== Reset PartialBytesAcked counter if no more outstanding bytes
        if (path->outstandingBytes == 0) {
            path->partialBytesAcked = 0;
        }
        path->vectorPathPbAcked->record(path->partialBytesAcked);
    }
    else {
        EV_INFO << assocId << ": " << simTime() << ":\tCC "
                << "Not increasing cwnd of path " << path->remoteAddress
                << " during Fast Recovery" << endl;
    }
}

void SCTPAssociation::cwndUpdateAfterRtxTimeout(SCTPPathVariables *path)
{
    double decreaseFactor = 0.5;
    path->oliaSentBytes = 0;
    EV << assocId << ": " << simTime() << ":\tCC [cwndUpdateAfterRtxTimeout]\t" << path->remoteAddress
       << " (cmtCCGroup=" << path->cmtCCGroup << ")"
       << "\tsst=" << path->ssthresh
       << "\tcwnd=" << path->cwnd
       << "\tSST=" << path->cmtGroupTotalSsthresh
       << "\tCWND=" << path->cmtGroupTotalCwnd
       << "\tBW.CWND=" << path->cmtGroupTotalCwndBandwidth;
    if (state->highSpeedCC == true) {
        decreaseFactor = HighSpeedCwndAdjustmentTable[path->highSpeedCCThresholdIdx].decreaseFactor;
        EV << "\tHighSpeedDecreaseFactor=" << decreaseFactor;
    }

    // ====== SCTP or CMT-SCTP (independent congestion control) ==============
    if ((state->allowCMT == false) || (state->cmtCCVariant == SCTPStateVariables::CCCV_CMT)) {
        path->ssthresh = max((int32)path->cwnd - (int32)rint(decreaseFactor * (double)path->cwnd),
                    4 * (int32)path->pmtu);
        path->cwnd = path->pmtu;
    }
    // ====== Resource Pooling RTX Timeout ===================================
    else {
        // ====== CMT/RPv1-SCTP RTX Timeout ===================================
        if (state->cmtCCVariant == SCTPStateVariables::CCCV_CMTRPv1) {
            const double sstRatio = (double)path->ssthresh / (double)path->cmtGroupTotalSsthresh;
            const int32 decreasedWindow = (int32)path->cwnd - (int32)rint(path->cmtGroupTotalCwnd * decreaseFactor);
            path->ssthresh = max(decreasedWindow,
                        max((int32)path->pmtu,
                                (int32)ceil((double)state->rpMinCwnd * (double)path->pmtu * sstRatio)));
            path->cwnd = max((int32)path->pmtu,
                        (int32)ceil((double)path->pmtu * sstRatio));
        }
        // ====== CMT/RPv2-SCTP RTX Timeout ===================================
        else if (state->cmtCCVariant == SCTPStateVariables::CCCV_CMTRPv2) {
            const double pathBandwidth = path->cwnd / GET_SRTT(path->srtt.dbl());
            const double bandwidthToGive = path->cmtGroupTotalCwndBandwidth / 2.0;
            const double reductionFactor = max(0.5, bandwidthToGive / pathBandwidth);

            path->ssthresh = (int32)max((int32)state->rpMinCwnd * (int32)path->pmtu,
                        (int32)ceil(path->cwnd - reductionFactor * path->cwnd));
            path->cwnd = path->pmtu;
        }
        // ====== Like MPTCP RTX Timeout ======================================
        else if(state->cmtCCVariant == SCTPStateVariables::CCCV_CMT_LIA) {
            path->ssthresh = max((int32)path->cwnd - (int32)rint(decreaseFactor * (double)path->cwnd),
                        (int32)state->rpMinCwnd * (int32)path->pmtu);
            path->cwnd = path->pmtu;
        }
        else if (state->cmtCCVariant == SCTPStateVariables::CCCV_CMT_OLIA) {
            // like draft
            path->ssthresh = max((int32) path->cwnd - (int32) rint(decreaseFactor * (double) path->cwnd),
                    4 * (int32) path->pmtu);
            path->cwnd = path->pmtu;
        }
        // ====== TEST RTX Timeout ============================================
        else if (state->cmtCCVariant == SCTPStateVariables::CCCV_CMTRP_Test1) {
            const double pathBandwidth = path->cwnd / GET_SRTT(path->srtt.dbl());
            const double bandwidthToGive = path->cmtGroupTotalCwndBandwidth / 2.0;
            const double reductionFactor = max(0.5, bandwidthToGive / pathBandwidth);

            path->ssthresh = (int32)max((int32)state->rpMinCwnd * (int32)path->pmtu,
                        (int32)ceil(path->cwnd - reductionFactor * path->cwnd));
            path->cwnd = path->pmtu;
        }
        // ====== Like MPTCP RTX Timeout ======================================
        else if (state->cmtCCVariant == SCTPStateVariables::CCCV_CMTRP_Test2) {
            path->ssthresh = max((int32)path->cwnd - (int32)rint(decreaseFactor * (double)path->cwnd),
                        (int32)state->rpMinCwnd * (int32)path->pmtu);
            path->cwnd = path->pmtu;
        }
        // ====== Other -> error ==============================================
        else {
            throw cRuntimeError("Implementation for this cmtCCVariant is missing!");
        }
    }
    path->highSpeedCCThresholdIdx = 0;
    path->partialBytesAcked = 0;
    path->vectorPathPbAcked->record(path->partialBytesAcked);
    EV_INFO << "\t=>\tsst=" << path->ssthresh
            << "\tcwnd=" << path->cwnd << endl;
    recordCwndUpdate(path);

    // Leave Fast Recovery mode
    if (path->fastRecoveryActive == true) {
        path->fastRecoveryActive = false;
        path->fastRecoveryExitPoint = 0;
        path->vectorPathFastRecoveryState->record(0);
    }
}

void SCTPAssociation::cwndUpdateMaxBurst(SCTPPathVariables *path)
{
    if ((state->maxBurstVariant == SCTPStateVariables::MBV_UseItOrLoseIt) ||
        (state->maxBurstVariant == SCTPStateVariables::MBV_CongestionWindowLimiting) ||
        (state->maxBurstVariant == SCTPStateVariables::MBV_UseItOrLoseItTempCwnd) ||
        (state->maxBurstVariant == SCTPStateVariables::MBV_CongestionWindowLimitingTempCwnd))
    {
        if (path->cwnd > ((path->outstandingBytes + state->maxBurst * path->pmtu))) {
            EV_INFO << assocId << ": " << simTime() << ":\tCC [cwndUpdateMaxBurst]\t"
                    << path->remoteAddress
                    << "\tsst=" << path->ssthresh
                    << "\tcwnd=" << path->cwnd
                    << "\ttempCwnd=" << path->tempCwnd
                    << "\tosb=" << path->outstandingBytes
                    << "\tmaxBurst=" << state->maxBurst * path->pmtu;

            // ====== Update cwnd or tempCwnd, according to MaxBurst variant ===
            if ((state->maxBurstVariant == SCTPStateVariables::MBV_UseItOrLoseIt) ||
                (state->maxBurstVariant == SCTPStateVariables::MBV_CongestionWindowLimiting))
            {
                path->cwnd = path->outstandingBytes + (state->maxBurst * path->pmtu);
            }
            else if ((state->maxBurstVariant == SCTPStateVariables::MBV_UseItOrLoseItTempCwnd) ||
                     (state->maxBurstVariant == SCTPStateVariables::MBV_CongestionWindowLimitingTempCwnd))
            {
                path->tempCwnd = path->outstandingBytes + (state->maxBurst * path->pmtu);
            }
            else {
                assert(false);
            }

            if (state->maxBurstVariant == SCTPStateVariables::MBV_CongestionWindowLimiting) {
                if (path->ssthresh < path->cwnd) {
                    path->ssthresh = path->cwnd;
                }
            }
            if (state->maxBurstVariant == SCTPStateVariables::MBV_CongestionWindowLimitingTempCwnd) {
                if (path->ssthresh < path->tempCwnd) {
                    path->ssthresh = path->tempCwnd;
                }
            }
            recordCwndUpdate(path);

            EV_INFO << "\t=>\tsst=" << path->ssthresh
                    << "\tcwnd=" << path->cwnd
                    << "\ttempCwnd=" << path->tempCwnd
                    << endl;
        }
        // ====== Possible transmission will not exceed burst size ============
        else {
            // Just store current cwnd to tempCwnd
            path->tempCwnd = path->cwnd;
        }
    }
}

void SCTPAssociation::cwndUpdateAfterCwndTimeout(SCTPPathVariables *path)
{
    // When the association does not transmit data on a given transport address
    // within an RTO, the cwnd of the transport address SHOULD be adjusted to 2*MTU.
    EV_INFO << assocId << ": " << simTime() << ":\tCC [cwndUpdateAfterCwndTimeout]\t" << path->remoteAddress
            << " (cmtCCGroup=" << path->cmtCCGroup << ")"
            << "\tsst=" << path->ssthresh
            << "\tcwnd=" << path->cwnd;
    path->cwnd = getInitialCwnd(path);
    EV_INFO << "\t=>\tsst=" << path->ssthresh
            << "\tcwnd=" << path->cwnd << endl;
    recordCwndUpdate(path);
}

} // namespace sctp

} // namespace inet

