#include <algorithm>    // min,max

#include "inet/transportlayer/tcp/TCP.h"
#include "inet/transportlayer/tcp/flavours/TCPWestwood.h"

namespace inet {

namespace tcp {

Register_Class(TCPWestwood);

TCPWestwoodStateVariables::TCPWestwoodStateVariables()
{
    ssthresh = 65535;
    w_RTTmin = 0x7fffffff;
    w_a = 1;
    w_lastAckTime = 0;
    w_bwe = 0;
    w_sample_bwe = 0;
}

TCPWestwoodStateVariables::~TCPWestwoodStateVariables()
{
}

std::string TCPWestwoodStateVariables::info() const
{
    std::stringstream out;
    out << TCPBaseAlgStateVariables::info();
    out << " ssthresh=" << ssthresh;
    return out.str();
}

std::string TCPWestwoodStateVariables::detailedInfo() const
{
    std::stringstream out;
    out << TCPBaseAlgStateVariables::detailedInfo();
    out << "ssthresh = " << ssthresh << "\n";
    out << "w_RTTmin = " << w_RTTmin << "\n";
    return out.str();
}

TCPWestwood::TCPWestwood()
    : TCPBaseAlg(), state((TCPWestwoodStateVariables *&)TCPAlgorithm::state)
{
}

void TCPWestwood::recalculateSlowStartThreshold()
{
    state->ssthresh = (uint32)((state->w_bwe * SIMTIME_DBL(state->w_RTTmin)) / (state->w_a));

    if (ssthreshVector)
        ssthreshVector->record(state->ssthresh);

    EV_DEBUG << "recalculateSlowStartThreshold(), ssthresh=" << state->ssthresh << "\n";
}

void TCPWestwood::recalculateBWE(uint32 cumul_ack)
{
    simtime_t currentTime = simTime();
    simtime_t timeAck = currentTime - state->w_lastAckTime;

    // Update BWE
    if (timeAck > 0) {
        double old_sample_bwe = state->w_sample_bwe;
        double old_bwe = state->w_bwe;
        state->w_sample_bwe = (cumul_ack) / timeAck;
        state->w_bwe = (19.0 / 21.0) * old_bwe + (1.0 / 21.0) * (state->w_sample_bwe + old_sample_bwe);
        EV_DEBUG << "recalculateBWE(), new w_bwe=" << state->w_bwe << "\n";
    }
    state->w_lastAckTime = currentTime;
}

void TCPWestwood::processRexmitTimer(TCPEventCode& event)
{
    TCPBaseAlg::processRexmitTimer(event);

    if (event == TCP_E_ABORT)
        return;

    // TCP Westwood: congestion control with faster recovery. S. Mascolo, C. Casetti, M. Gerla, S.S. Lee, M. Sanadidi
    // After REXMIT timeout in TCP Westwood: a increases from 1 to 4, in steps of 1 during slow start,
    // and is set to 1 in cong. avoidance.
    // the cong. window is reset to 1 after a timeout, as is done by TCP Reno. Conservative. Reaseon: fairness.

    if (state->snd_cwnd < state->ssthresh) {    // Slow start
        state->w_a = state->w_a + 1;
        if (state->w_a > 4)
            state->w_a = 4;
    }
    else {    // Cong. avoidance
        state->w_a = 1;
    }

    recalculateSlowStartThreshold();
    if (state->ssthresh < 2 * state->snd_mss)
        state->ssthresh = 2 * state->snd_mss;

    state->snd_cwnd = state->snd_mss;

    if (cwndVector)
        cwndVector->record(state->snd_cwnd);

    state->afterRto = true;
    conn->retransmitOneSegment(true);
}

void TCPWestwood::receivedDataAck(uint32 firstSeqAcked)
{
    TCPBaseAlg::receivedDataAck(firstSeqAcked);

    const TCPSegmentTransmitInfoList::Item *found = state->regions.get(firstSeqAcked);
    state->regions.clearTo(state->snd_una);

    if (found != NULL) {
        simtime_t currentTime = simTime();
        simtime_t newRTT = currentTime - found->getFirstSentTime();

        // Update RTTmin
        if (newRTT < state->w_RTTmin && newRTT > 0 && found->getTransmitCount() == 1)
            state->w_RTTmin = newRTT;

        // cumul_ack: cumulative ack's that acks 2 or more pkts count 1,
        // because DUPACKs count them
        uint32 cumul_ack = state->snd_una - firstSeqAcked;    // acked bytes
        if ((state->dupacks * state->snd_mss) >= cumul_ack)
            cumul_ack = state->snd_mss; // cumul_ack = 1:
        else
            cumul_ack -= (state->dupacks * state->snd_mss);

        // security check: if previous steps are right cumul_ack shoudl be > 2:
        if (cumul_ack > (2 * state->snd_mss))
            cumul_ack = 2 * state->snd_mss;

        recalculateBWE(cumul_ack);
    }    // Closes if w_sendtime != NULL

    // Same behavior of Reno during fast recovery, slow start and cong. avoidance

    if (state->dupacks >= DUPTHRESH) {    // DUPTHRESH = 3
        //
        // Perform Fast Recovery: set cwnd to ssthresh (deflating the window).
        //
        EV_DETAIL << "Fast Recovery: setting cwnd to ssthresh=" << state->ssthresh << "\n";
        state->snd_cwnd = state->ssthresh;

        if (cwndVector)
            cwndVector->record(state->snd_cwnd);
    }
    else {
        //
        // Perform slow start and congestion avoidance.
        //
        if (state->snd_cwnd < state->ssthresh) {
            EV_DETAIL << "cwnd <= ssthresh: Slow Start: increasing cwnd by one SMSS bytes to ";

            // perform Slow Start. RFC 2581: "During slow start, a TCP increments cwnd
            // by at most SMSS bytes for each ACK received that acknowledges new data."
            state->snd_cwnd += state->snd_mss;

            // Note: we could increase cwnd based on the number of bytes being
            // acknowledged by each arriving ACK, rather than by the number of ACKs
            // that arrive. This is called "Appropriate Byte Counting" (ABC) and is
            // described in RFC 3465. This RFC is experimental and probably not
            // implemented in real-life TCPs, hence it's commented out. Also, the ABC
            // RFC would require other modifications as well in addition to the
            // two lines below.
            //
            // int bytesAcked = state->snd_una - firstSeqAcked;
            // state->snd_cwnd += bytesAcked * state->snd_mss;

            if (cwndVector)
                cwndVector->record(state->snd_cwnd);

            EV_DETAIL << "cwnd=" << state->snd_cwnd << "\n";
        }
        else {
            // perform Congestion Avoidance (RFC 2581)
            uint32 incr = state->snd_mss * state->snd_mss / state->snd_cwnd;

            if (incr == 0)
                incr = 1;

            state->snd_cwnd += incr;

            if (cwndVector)
                cwndVector->record(state->snd_cwnd);

            //
            // Note: some implementations use extra additive constant mss / 8 here
            // which is known to be incorrect (RFC 2581 p5)
            //
            // Note 2: RFC 3465 (experimental) "Appropriate Byte Counting" (ABC)
            // would require maintaining a bytes_acked variable here which we don't do
            //

            EV_DETAIL << "cwnd > ssthresh: Congestion Avoidance: increasing cwnd linearly, to " << state->snd_cwnd << "\n";
        }
    }

    sendData(false);
}

void TCPWestwood::receivedDuplicateAck()
{
    TCPBaseAlg::receivedDuplicateAck();

    {
        // BWE calculation: dupack counts 1
        uint32 cumul_ack = state->snd_mss;
        recalculateBWE(cumul_ack);
    }    // Closes if w_sendtime != NULL

    if (state->dupacks == DUPTHRESH) {    // DUPTHRESH = 3
        EV_DETAIL << "Westwood on dupAcks == DUPTHRESH(=3): Faster Retransmit \n";

        // TCP Westwood: congestion control with faster recovery. S. Mascolo, C. Casetti, M. Gerla, S.S. Lee, M. Sanadidi
        // During the cong. avoidance phase we are probing for extra available bandwidth.
        // Therefore, when n DUPACKS are received, it means that we have hit the network
        // capacity. Thus, the slow start threshold is set equal to the available pipe size
        // (BWE*RTTmin), the cong. window is set equal to ssthresh and the cong. avoidance phase
        // is entered again to gently probe for new available bandwitdh.

        // During the slow start phase we are still probing for the available bandwidth.
        // Therefore the BWE we obtain after n duplicate ACKs is used to set the slow start threshold.
        // After ssthresh has been set, the cong. window is set equal to the slow start theshold only
        // if cwin>ssthresh. In other words, during slow start, cwin still features an exponential
        // increase as in the current implementation of TCP Reno.

        // a increases from 1 to 4 in steps of 0.25 every time 3 DUPACKS are received in slow start.
        // while is set to 1 in cong. avoidance. (is initialized as 1).
        // The purpose of the theshold reduction factor a is to dampen a possible overestimation of
        // the available bandwidth. the more frequently a triple DUPACK is received during slow start,
        // the bigger the reduction factor becomes.
        // a is restored to 1 in cong. avoidance: ssthresh was set correctly and there is no need to reduce
        // the impact of BWE

        if (state->snd_cwnd < state->ssthresh) {    // Slow start
            state->w_a = state->w_a + 0.25;
            if (state->w_a > 4)
                state->w_a = 4;
        }
        else {    // Cong. avoidance
            state->w_a = 1;
        }

        recalculateSlowStartThreshold();
        // reset cwnd to ssthresh, if larger
        if (state->snd_cwnd > state->ssthresh)
            state->snd_cwnd = state->ssthresh;

        if (cwndVector)
            cwndVector->record(state->snd_cwnd);

        EV_DETAIL << " set cwnd=" << state->snd_cwnd << ", ssthresh=" << state->ssthresh << "\n";

        // Fast Retransmission: retransmit missing segment without waiting
        // for the REXMIT timer to expire
        restartRexmitTimer();
        conn->retransmitOneSegment(false);

        sendData(false);
    }
    // Behavior like Reno:
    else if (state->dupacks > DUPTHRESH) {    // DUPTHRESH = 3
        // Westwood: like Reno
        state->snd_cwnd += state->snd_mss;
        EV_DETAIL << "Westwood on dupAcks > DUPTHRESH(=3): Fast Recovery: inflating cwnd by SMSS, new cwnd=" << state->snd_cwnd << "\n";

        if (cwndVector)
            cwndVector->record(state->snd_cwnd);

        sendData(false);
    }
}

void TCPWestwood::dataSent(uint32 fromseq)
{
    TCPBaseAlg::dataSent(fromseq);

    // save time when packet is sent
    // fromseq is the seq number of the 1st sent byte

    simtime_t sendtime = simTime();
    state->regions.clearTo(state->snd_una);
    state->regions.set(fromseq, state->snd_max, sendtime);
}

void TCPWestwood::segmentRetransmitted(uint32 fromseq, uint32 toseq)
{
    TCPBaseAlg::segmentRetransmitted(fromseq, toseq);

    state->regions.set(fromseq, toseq, simTime());
}

} // namespace tcp

} // namespace inet

