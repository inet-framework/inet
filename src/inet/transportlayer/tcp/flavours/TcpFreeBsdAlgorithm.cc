//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/transportlayer/tcp/flavours/TcpFreeBsdAlgorithm.h"

#include "inet/transportlayer/tcp/flavours/freebsd/cc_cubic.h"
#include "inet/transportlayer/tcp/flavours/freebsd/cc_newreno.h"
#include "inet/transportlayer/tcp/flavours/freebsd/tcp_var.h"
#include "inet/transportlayer/tcp/flavours/Rfc5681Recovery.h"
#include "inet/transportlayer/tcp/TcpSackRexmitQueue.h"

namespace inet {
namespace tcp {

Register_Class(TcpFreeBsdAlgorithm);

// TODO call these somewhere
//after_idle(struct cc_var *ccv);
//newround(struct cc_var *ccv, uint32_t round_cnt);
//rttsample(struct cc_var *ccv, uint32_t usec_rtt, uint32_t rxtcnt, uint32_t fas);

TcpFreeBsdAlgorithm::TcpFreeBsdAlgorithm()
    : TcpAlgorithmBase(),
      state((TcpClassicAlgorithmBaseStateVariables *&)TcpAlgorithm::state)
{
}

void TcpFreeBsdAlgorithm::initialize()
{
    TcpAlgorithmBase::initialize();
    state->ssthresh = conn->getTcpMain()->par("initialSsthresh");
    //algo = newreno_cc_algo;
    algo = cubic_cc_algo;
    algo.cb_init(&cc, nullptr);
    cc.flags = CCF_CWND_LIMITED | CUBICFLAG_HYSTART_ENABLED | CUBICFLAG_HYSTART_IN_CSS;
    cc.ccvc.tcp = new tcpcb();
}

void TcpFreeBsdAlgorithm::established(bool active)
{
    TcpAlgorithmBase::established(active);
    if (algo.conn_init != nullptr)
        algo.conn_init(&cc);
}

void TcpFreeBsdAlgorithm::rttMeasurementComplete(simtime_t tSent, simtime_t tAcked)
{
    TcpAlgorithmBase::rttMeasurementComplete(tSent, tAcked);
    simtime_t rtt = tAcked - tSent;
    algo.rttsample(&cc, rtt.inUnit(SIMTIME_US), 1, 0);
    auto tcp = cc.ccvc.tcp;
    tcp->t_rttupdated++;
}

//void TcpFreeBsdAlgorithm::processRexmitTimer(TcpEventCode& event)
//{
//}

void TcpFreeBsdAlgorithm::receivedAckForAlreadyAckedData(const TcpHeader *tcpHeader, uint32_t payloadLength)
{
    Rfc5681Recovery recovery(state, conn);
    if (recovery.isDuplicateAck(tcpHeader, payloadLength)) {
        if (!state->lossRecovery) {
            state->dupacks++;
            conn->emit(dupAcksSignal, state->dupacks);
        }
        receivedDuplicateAck();
    }
    else {
        state->dupacks = 0;
        conn->emit(dupAcksSignal, state->dupacks);
    }
}

void TcpFreeBsdAlgorithm::copythere()
{
    auto tcp = cc.ccvc.tcp;
    tcp->t_maxseg = state->snd_mss;
    tcp->t_effmaxseg = state->snd_effmss;
    tcp->snd_scale = state->snd_wnd_scale;
    tcp->snd_cwnd = state->snd_cwnd;
    tcp->snd_max = state->snd_max;
    tcp->snd_nxt = state->snd_nxt;
    tcp->snd_ssthresh = state->ssthresh;
    tcp->snd_wnd = state->snd_wnd;
    if (state->lossRecovery)
        ENTER_RECOVERY(tcp->t_flags);
    else
        EXIT_RECOVERY(tcp->t_flags);
    //    tcp->t_flags;
    //    tcp->t_rttupdated;
    //    tcp->t_rxtshift;
    //    tcp->t_inpcb;
    tcp->t_srtt = state->srtt.inUnit(SIMTIME_US);
    tcp->flightSize = getBytesInFlight();
    ticks = simTime().inUnit(SIMTIME_US) / tick;
}

void TcpFreeBsdAlgorithm::copyback()
{
    auto tcp = cc.ccvc.tcp;
    state->snd_mss = tcp->t_maxseg;
    state->snd_effmss = tcp->t_effmaxseg;
    state->snd_wnd_scale = tcp->snd_scale;
    if (state->snd_cwnd != tcp->snd_cwnd) {
        state->snd_cwnd = tcp->snd_cwnd;
        conn->emit(cwndSignal, state->snd_cwnd);
    }
    state->snd_max = tcp->snd_max;
    state->snd_nxt = tcp->snd_nxt;
    if (state->ssthresh != tcp->snd_ssthresh) {
        state->ssthresh = tcp->snd_ssthresh;
        conn->emit(ssthreshSignal, state->ssthresh);
    }
    state->snd_wnd = tcp->snd_wnd;
    state->lossRecovery = IN_RECOVERY(tcp->t_flags);
    //    tcp->t_flags;
    //    tcp->t_rttupdated;
    //    tcp->t_rxtshift;
    //    tcp->t_inpcb;
    state->srtt = tcp->t_srtt / 1E+6;
}

void TcpFreeBsdAlgorithm::receivedAckForUnackedData(uint32_t firstSeqAcked)
{
    TcpAlgorithmBase::receivedAckForUnackedData(firstSeqAcked);
    cc.bytes_this_ack = state->snd_una - firstSeqAcked;
    if (state->lossRecovery) {
        if (seqGE(state->snd_una - 1, state->recover)) {
            copythere();
            algo.post_recovery(&cc);
            copyback();
//            state->snd_cwnd = state->ssthresh; // use option (2)
//            conn->emit(cwndSignal, state->ssthresh);
            state->lossRecovery = false;
            state->firstPartialACK = false;
            EV_INFO << "Loss recovery terminated" << EV_ENDL;
        }
        else {
            conn->retransmitOneSegment(false);
            conn->sendData(state->snd_cwnd);
            if (!state->firstPartialACK) {
                state->firstPartialACK = true;
                EV_DETAIL << "First partial ACK arrived during recovery, restarting REXMIT timer.\n";
                conn->getTcpAlgorithmForUpdate()->restartRexmitTimer();
            }
        }
    }
    if (!state->lossRecovery) {
        copythere();
        cc.nsegs = 1;
        algo.ack_received(&cc, CC_ACK);
        copyback();
    }
    sendData(false);
}

//void TcpFreeBsdAlgorithm::receivedAckForUnsentData(uint32_t seq)
//{
//}

void TcpFreeBsdAlgorithm::receivedDuplicateAck()
{
    if (state->dupacks == state->dupthresh) {
        conn->getRexmitQueueForUpdate()->markHeadLost(); // update for flight size calculation
        if (!state->lossRecovery) {
            state->recover = state->snd_max - 1;

            copythere();
            algo.cong_signal(&cc, CC_NDUPACK);
            algo.newround(&cc, 0);
            auto tcp = cc.ccvc.tcp;
            tcp->snd_cwnd = tcp->snd_ssthresh;
    //        tcp->snd_cwnd = tcp->snd_ssthresh + maxseg * (tcp->t_dupacks - tcp->snd_limited);
            copyback();

            conn->retransmitOneSegment(false);
        }
    }
    sendData(false);
}

uint32_t TcpFreeBsdAlgorithm::getBytesInFlight() const
{
    auto rexmitQueue = conn->getRexmitQueue();
    int64_t sentSize = state->snd_max - state->snd_una;
    int64_t in_flight = sentSize - rexmitQueue->getSacked() - rexmitQueue->getLost() + rexmitQueue->getRetrans();
    if (in_flight < 0)
        in_flight = 0;
    conn->emit(bytesInFlightSignal, in_flight);
    return in_flight;
}

} // namespace tcp
} // namespace inet
