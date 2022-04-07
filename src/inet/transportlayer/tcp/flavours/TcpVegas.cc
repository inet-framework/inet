//
// Copyright (C) 2013 Maria Fernandez, Carlos Calafate, Juan-Carlos Cano and Pietro Manzoni
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/transportlayer/tcp/flavours/TcpVegas.h"

#include <algorithm> // min,max

#include "inet/transportlayer/tcp/Tcp.h"

namespace inet {

namespace tcp {

Register_Class(TcpVegas);

std::string TcpVegasStateVariables::str() const
{
    std::stringstream out;
    out << TcpBaseAlgStateVariables::str();
    out << " ssthresh=" << ssthresh;
    return out.str();
}

std::string TcpVegasStateVariables::detailedInfo() const
{
    std::stringstream out;
    out << TcpBaseAlgStateVariables::detailedInfo();
    out << "ssthresh = " << ssthresh << "\n";
    out << "baseRTT = " << v_baseRTT << "\n";
    return out.str();
}

TcpVegas::TcpVegas()
    : TcpBaseAlg(), state((TcpVegasStateVariables *&)TcpAlgorithm::state)
{
}

// Same as TcpReno
void TcpVegas::recalculateSlowStartThreshold()
{
    // RFC 2581, page 4:
    // "When a TCP sender detects segment loss using the retransmission
    // timer, the value of ssthresh MUST be set to no more than the value
    // given in equation 3:
    //
    //   ssthresh = max (FlightSize / 2, 2*SMSS)            (3)
    //
    // As discussed above, FlightSize is the amount of outstanding data in
    // the network."

    // set ssthresh to flight size/2, but at least 2 SMSS
    // (the formula below practically amounts to ssthresh=cwnd/2 most of the time)
    uint32_t flight_size = std::min(state->snd_cwnd, state->snd_wnd); // FIXME - Does this formula computes the amount of outstanding data?
//    uint32_t flight_size = state->snd_max - state->snd_una;
    state->ssthresh = std::max(flight_size / 2, 2 * state->snd_mss);
    conn->emit(ssthreshSignal, state->ssthresh);
}

// Process rexmit timer
void TcpVegas::processRexmitTimer(TcpEventCode& event)
{
    TcpBaseAlg::processRexmitTimer(event);

    if (event == TCP_E_ABORT)
        return;

    recalculateSlowStartThreshold();
    // Vegas: when rtx timeout: cwnd = 2*smss, instead of 1*smss (Reno)
    state->snd_cwnd = 2 * state->snd_mss;

    conn->emit(cwndSignal, state->snd_cwnd);
    EV_DETAIL << "RXT Timeout in Vegas: resetting cwnd to " << state->snd_cwnd << "\n"
              << ", ssthresh=" << state->ssthresh << "\n";

    state->v_cwnd_changed = simTime(); // Save time when cwnd changes due to rtx

    state->afterRto = true;

    conn->retransmitOneSegment(true); // retransmit one segment from snd_una
}

void TcpVegas::receivedDataAck(uint32_t firstSeqAcked)
{
    TcpBaseAlg::receivedDataAck(firstSeqAcked);

    const TcpSegmentTransmitInfoList::Item *found = state->regions.get(firstSeqAcked);
    if (found) {
        simtime_t currentTime = simTime();
        simtime_t tSent = found->getFirstSentTime();
        int num_transmits = found->getTransmitCount();

        // TODO When should do it: when received first ACK, or when received ACK of 1st sent packet???
        if (firstSeqAcked == state->iss + 1) { // Initialization
            state->v_baseRTT = currentTime - tSent;
            state->v_sa = state->v_baseRTT * 8;
            state->v_sd = state->v_baseRTT;
            state->v_rtt_timeout = ((state->v_sa / 4.0) + state->v_sd) / 2.0;
            EV_DETAIL << "Vegas: initialization" << "\n";
        }

        // Once per RTT
        if (seqGreater(state->snd_una, state->v_begseq)) {
            simtime_t newRTT;
            if (state->v_cntRTT > 0) {
                newRTT = state->v_sumRTT / state->v_cntRTT;
                EV_DETAIL << "Vegas: newRTT (state->v_sumRTT / state->v_cntRTT) calculated: " << state->v_sumRTT / state->v_cntRTT << "\n";
            }
            else {
                newRTT = currentTime - state->v_begtime;
                EV_DETAIL << "Vegas: newRTT calculated: " << newRTT << "\n";
            }
            state->v_sumRTT = 0.0;
            state->v_cntRTT = 0;

            // decide if incr/decr cwnd
            if (newRTT > 0) {
                // rttLen: bytes transmitted since a segment is sent and its ACK is received
                uint32_t rttLen = state->snd_nxt - state->v_begseq;

                // If only one packet in transit: update baseRTT
                if (rttLen <= state->snd_mss)
                    state->v_baseRTT = newRTT;

                // actual = rttLen/(current_rtt)
                uint32_t actual = rttLen / newRTT;

                // expected = (current window size)/baseRTT
                uint32_t expected;
                uint32_t acked = state->snd_una - firstSeqAcked;
                expected = (uint32_t)((state->snd_nxt - firstSeqAcked) + std::min(state->snd_mss - acked, (uint32_t)0)) / state->v_baseRTT;

                // diff = expected - actual
                uint32_t diff = (uint32_t)((expected - actual) * SIMTIME_DBL(state->v_baseRTT) + 0.5);

                EV_DETAIL << "Vegas: expected: " << expected << "\n"
                          << ", actual =" << actual << "\n"
                          << ", diff =" << diff << "\n";

                // Slow start
                state->v_incr_ss = false; // reset
                if (state->snd_cwnd < state->ssthresh) { // Slow start
                    EV_DETAIL << "Vegas: Slow Start: " << "\n";

                    // cwnd modification during slow-start only every 2 rtt
                    state->v_inc_flag = !state->v_inc_flag;
                    if (!state->v_inc_flag) {
                        state->v_incr = 0;
                    }
                    else {
                        if (diff > 1 * state->snd_mss) { // gamma
                            /* When actual rate falls below expected rate a certain value (gamma threshold)
                               Vegas changes from slow start to linear incr/decr */

                            recalculateSlowStartThreshold(); // to enter again in cong. avoidance
                            state->snd_cwnd -= (state->snd_cwnd / 8);

                            if (state->snd_cwnd < 2 * state->snd_mss)
                                state->snd_cwnd = 2 * state->snd_mss;

                            state->v_incr = 0;

                            conn->emit(cwndSignal, state->snd_cwnd);
                        }
                        else {
                            state->v_incr = state->snd_mss; // incr 1 segment
                            state->v_incr_ss = true;
                        }
                    }
                } // end slow start
                else {
                    // Cong. avoidance
                    EV_DETAIL << "Vegas: Congestion avoidance: " << "\n";

                    if (diff > 4 * state->snd_mss) { // beta
                        state->v_incr = -state->snd_mss;
                    }
                    else if (diff < 2 * state->snd_mss) { // alpha
                        state->v_incr = state->snd_mss;
                    }
                    else
                        state->v_incr = 0;
                } // end cong. avoidance
            }
            // register beqseq and begtime values for next rtt
            state->v_begtime = currentTime;
            state->v_begseq = state->snd_nxt;
        } // end 'once per rtt' section

        // if cwnd > ssthresh, no incr
        if (state->v_incr_ss && state->snd_cwnd >= state->ssthresh) {
            state->v_incr = 0;
            EV_DETAIL << "Vegas: surpass ssthresh during slow-start: no cwnd incr. " << "\n";
        }

        // incr/decr cwnd
        if (state->v_incr > 0) {
            state->snd_cwnd += state->v_incr;

            conn->emit(cwndSignal, state->snd_cwnd);
            EV_DETAIL << "Vegas: incr cwnd linearly, to " << state->snd_cwnd << "\n";
        }
        else if (state->v_incr < 0) {
            state->snd_cwnd += state->v_incr;
            if (state->snd_cwnd < 2 * state->snd_mss)
                state->snd_cwnd = 2 * state->snd_mss;

            conn->emit(cwndSignal, state->snd_cwnd);
            EV_DETAIL << "Vegas: decr cwnd linearly, to " << state->snd_cwnd << "\n";
        }

        // update vegas fine-grained timeout value (retransmitted packets do not count)
        if (tSent != 0 && num_transmits == 1) {
            simtime_t newRTT = currentTime - tSent;
            state->v_sumRTT += newRTT;
            ++state->v_cntRTT;

            if (newRTT > 0) {
                if (newRTT < state->v_baseRTT)
                    state->v_baseRTT = newRTT;

                simtime_t n = newRTT - state->v_sa / 8;
                state->v_sa += n;
                n = n < 0 ? -n : n;
                n -= state->v_sd / 4;
                state->v_sd += n;
                state->v_rtt_timeout = ((state->v_sa / 4) + state->v_sd) / 2;
                state->v_rtt_timeout += (state->v_rtt_timeout / 16);
                EV_DETAIL << "Vegas: new v_rtt_timeout = " << state->v_rtt_timeout << "\n";
            }
        }

        // check 1st and 2nd ack after a rtx
        if (state->v_worried > 0) {
            state->v_worried -= state->snd_mss;
            const TcpSegmentTransmitInfoList::Item *unaFound = state->regions.get(state->snd_una);
//            bool expired = unaFound && ((currentTime - unaFound->getLastSentTime()) >= state->v_rtt_timeout);
            bool expired = unaFound && ((currentTime - unaFound->getFirstSentTime()) >= state->v_rtt_timeout);

            // added comprobation to check that received ACK do not acks all outstanding data. If not,
            // TcpConnection::retransmitOneSegment will fail: ASSERT(bytes!=0), line 839), because bytes = snd_max-snd_una
            if (expired && (state->snd_max - state->snd_una > 0)) {
                state->dupacks = state->dupthresh;
                EV_DETAIL << "Vegas: retransmission (v_rtt_timeout) " << "\n";
                conn->retransmitOneSegment(false); // retransmit one segment from snd_una
            }
            else
                state->v_worried = 0;
        }
    } // Closes if v_sendtime != nullptr

    state->regions.clearTo(state->snd_una);

    // Try to send more data
    sendData(false);
}

void TcpVegas::receivedDuplicateAck()
{
    TcpBaseAlg::receivedDuplicateAck();

    simtime_t currentTime = simTime();
    simtime_t tSent = 0;
    int num_transmits = 0;
    const TcpSegmentTransmitInfoList::Item *found = state->regions.get(state->snd_una);
    if (found) {
        tSent = found->getFirstSentTime();
        num_transmits = found->getTransmitCount();
    }
    state->regions.clearTo(state->snd_una);

    // check Vegas timeout
    bool expired = found && ((currentTime - tSent) >= state->v_rtt_timeout);

    // rtx if Vegas timeout || 3 dupacks
    if (expired || state->dupacks == state->dupthresh) {
        uint32_t win = std::min(state->snd_cwnd, state->snd_wnd);
        state->v_worried = std::min((uint32_t)2 * state->snd_mss, state->snd_nxt - state->snd_una);

        if (found) {
            if (num_transmits > 1)
                state->v_rtt_timeout *= 2; // exp. Backoff
            else
                state->v_rtt_timeout += (state->v_rtt_timeout / 8.0);

            // Vegas reduces cwnd if retransmitted segment rtx. was sent after last cwnd reduction
            if (state->v_cwnd_changed < tSent) {
                win = win / state->snd_mss;
                if (win <= 3)
                    win = 2;
                else if (num_transmits > 1)
                    win = win / 2; // win <<= 1
                else
                    win -= win / 4; // win -= (win >>2)

                state->snd_cwnd = win * state->snd_mss + 3 * state->snd_mss;
                state->v_cwnd_changed = currentTime;

                conn->emit(cwndSignal, state->snd_cwnd);

                // reset rtx. timer
                restartRexmitTimer();
            }
        }

        // retransmit one segment from snd_una
        conn->retransmitOneSegment(false);

        if (found && num_transmits == 1)
            state->dupacks = state->dupthresh;
    }
    // else if dupacks > duphtresh, cwnd+1
    else if (state->dupacks > state->dupthresh) {
        state->snd_cwnd += state->snd_mss;
        conn->emit(cwndSignal, state->snd_cwnd);
    }

    // try to send more data
    sendData(false);
}

void TcpVegas::dataSent(uint32_t fromseq)
{
    TcpBaseAlg::dataSent(fromseq);

    // save time when packet is sent
    // fromseq is the seq number of the 1st sent byte
    // we need this value, based on iss=0 (to store it the right way on the vector),
    // but iss is not a constant value (ej: iss=0), so it needs to be detemined each time
    // (this is why it is used: fromseq-state->iss)

    state->regions.clearTo(state->snd_una);
    state->regions.set(fromseq, state->snd_max, simTime());
}

void TcpVegas::segmentRetransmitted(uint32_t fromseq, uint32_t toseq)
{
    TcpBaseAlg::segmentRetransmitted(fromseq, toseq);

    state->regions.set(fromseq, toseq, simTime());
}

} // namespace tcp

} // namespace inet

