/*
 * Copyright (c) 2014 Natale Patriciello <natale.patriciello@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "inet/transportlayer/tcp/flavours/TcpCubic.h"

#include "inet/transportlayer/tcp/flavours/Rfc6582Recovery.h"
#include "inet/transportlayer/tcp/TcpSackRexmitQueue.h"

namespace inet {
namespace tcp {

Register_Class(TcpCubic);

TcpCubic::TcpCubic()
    : TcpAlgorithmBase(),
      state((TcpClassicAlgorithmBaseStateVariables *&)TcpAlgorithm::state),
      m_cWndCnt(0),
      m_lastMaxCwnd(0),
      m_bicOriginPoint(0),
      m_bicK(0.0),
      m_delayMin(-1),
      m_epochStart(-1),
      m_found(false),
      m_roundStart(-1),
      m_endSeq(0),
      m_lastAck(-1),
      m_cubicDelta(-1),
      m_currRtt(-1),
      m_sampleCnt(0)
{
}

void TcpCubic::initialize()
{
    TcpAlgorithmBase::initialize();
    state->ssthresh = conn->getTcpMain()->par("initialSsthresh");
    m_fastConvergence = true;
    m_beta = 0.7;
    m_hystart = true;
    m_hystartLowWindow = 16;
    m_hystartDetect = HybridSSDetectionMode::BOTH;
    m_hystartMinSamples = 8;
    m_hystartAckDelta = 2E-3;
    m_hystartDelayMin = 4E-3;
    m_hystartDelayMax = 1;
    m_cubicDelta = 10E-3;
    m_cntClamp = 20;
    m_c = 0.4;
}

void TcpCubic::established(bool active)
{
    TcpAlgorithmBase::established(active);
    recovery = new Rfc6582Recovery(state, conn);
}

void TcpCubic::processRexmitTimer(TcpEventCode& event)
{
    TcpAlgorithmBase::processRexmitTimer(event);

    if (event == TCP_E_ABORT)
        return;

//    // RFC 3782, page 6:
//    // "6)  Retransmit timeouts:
//    // After a retransmit timeout, record the highest sequence number
//    // transmitted in the variable "recover" and exit the Fast Recovery
//    // procedure if applicable."
//    state->recover = (state->snd_max - 1);
//    EV_INFO << "recover=" << state->recover << "\n";
//    state->lossRecovery = false;
//    state->firstPartialACK = false;
//    EV_INFO << "Loss Recovery terminated.\n";

    state->ssthresh = calculateSsthresh(conn->getTcpAlgorithm()->getBytesInFlight());
    conn->emit(ssthreshSignal, state->ssthresh);

    state->snd_cwnd = state->snd_effmss;
    conn->emit(cwndSignal, state->snd_cwnd);

    EV_INFO << "Begin Slow Start: resetting cwnd to " << state->snd_cwnd
            << ", ssthresh=" << state->ssthresh << "\n";
    state->afterRto = true;
    conn->retransmitOneSegment(true);

    cubicReset();
    hystartReset();
}

void TcpCubic::receivedAckForAlreadyAckedData(const TcpHeader *tcpHeader, uint32_t payloadLength)
{
    TcpAlgorithmBase::receivedAckForAlreadyAckedData(tcpHeader, payloadLength);
    if (recovery->isDuplicateAck(tcpHeader, payloadLength)) {
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
    pktsAcked(1, state->srtt);
}

void TcpCubic::receivedAckForUnackedData(uint32_t firstSeqAcked)
{
    TcpAlgorithmBase::receivedAckForUnackedData(firstSeqAcked);
    uint32_t numBytesAcked = state->snd_una - firstSeqAcked;
    uint32_t numSegmentsAcked = numBytesAcked / state->snd_effmss;
    pktsAcked(numSegmentsAcked, state->srtt);
    if (state->lossRecovery)
        recovery->receivedAckForUnackedData(numBytesAcked);
    else
        increaseWindow(numSegmentsAcked);
    sendData(false);
}

void TcpCubic::receivedAckForUnsentData(uint32_t seq)
{
    TcpAlgorithmBase::receivedAckForUnsentData(seq);
}

void TcpCubic::receivedDuplicateAck()
{
    recovery->receivedDuplicateAck();
}

uint32_t TcpCubic::getBytesInFlight() const
{
    auto rexmitQueue = conn->getRexmitQueue();
    int64_t sentSize = state->snd_max - state->snd_una;
    int64_t in_flight = sentSize - rexmitQueue->getSacked() - rexmitQueue->getLost() + rexmitQueue->getRetrans();
    if (in_flight < 0)
        in_flight = 0;
    conn->emit(bytesInFlightSignal, in_flight);
    return in_flight;
}

void TcpCubic::hystartReset()
{
    m_roundStart = m_lastAck = simTime();
    m_endSeq = state->snd_max;
    m_currRtt = -1;
    m_sampleCnt = 0;
}

void TcpCubic::increaseWindow(uint32_t segmentsAcked)
{
    if (state->snd_cwnd < state->ssthresh) {
        if (m_hystart && seqGreater(state->snd_una, m_endSeq))
            hystartReset();

        // In Linux, the QUICKACK socket option enables the receiver to send
        // immediate acks initially (during slow start) and then transition
        // to delayed acks.  ns-3 does not implement QUICKACK, and if ack
        // counting instead of byte counting is used during slow start window
        // growth, when TcpSocket::DelAckCount==2, then the slow start will
        // not reach as large of an initial window as in Linux.  Therefore,
        // we can approximate the effect of QUICKACK by making this slow
        // start phase perform Appropriate Byte Counting (RFC 3465)
        state->snd_cwnd += segmentsAcked * state->snd_effmss;
        conn->emit(cwndSignal, state->snd_cwnd);
        segmentsAcked = 0;

        EV_INFO << "In SlowStart, updated to cwnd " << state->snd_cwnd << " ssthresh " << state->ssthresh << EV_ENDL;
    }

    if (state->snd_cwnd >= state->ssthresh && segmentsAcked > 0) {
        m_cWndCnt += segmentsAcked;
        uint32_t cnt = update();

        /* According to RFC 6356 even once the new cwnd is
         * calculated you must compare this to the number of ACKs received since
         * the last cwnd update. If not enough ACKs have been received then cwnd
         * cannot be updated.
         */
        if (m_cWndCnt >= cnt) {
            state->snd_cwnd += state->snd_effmss;
            conn->emit(cwndSignal, state->snd_cwnd);
            m_cWndCnt -= cnt;
            EV_INFO << "In CongAvoid, updated to cwnd " << state->snd_cwnd << EV_ENDL;
        }
        else
            EV_INFO << "Not enough segments have been ACKed to increment cwnd. Until now " << m_cWndCnt << " cnd " << cnt << EV_ENDL;
    }
}

uint32_t TcpCubic::update()
{
    simtime_t t;
    uint32_t delta;
    uint32_t bicTarget;
    uint32_t cnt = 0;
    double offs;
    uint32_t segCwnd = state->snd_cwnd / state->snd_effmss;

    if (m_epochStart == -1) {
        m_epochStart = simTime(); // record the beginning of an epoch

        if (m_lastMaxCwnd <= segCwnd) {
            EV_DEBUG << "lastMaxCwnd <= m_cWnd. K=0 and origin=" << segCwnd << EV_ENDL;
            m_bicK = 0.0;
            m_bicOriginPoint = segCwnd;
        }
        else {
            m_bicK = std::pow((m_lastMaxCwnd - segCwnd) / m_c, 1 / 3.);
            m_bicOriginPoint = m_lastMaxCwnd;
            EV_DEBUG << "lastMaxCwnd > m_cWnd. K=" << m_bicK << " and origin=" << m_lastMaxCwnd << EV_ENDL;
        }
    }

    t = simTime() + m_delayMin - m_epochStart;

    if (t.dbl() < m_bicK) /* t - K */ {
        offs = m_bicK - t.dbl();
        EV_DEBUG << "t=" << t.dbl() << " <k: offs=" << offs << EV_ENDL;
    }
    else {
        offs = t.dbl() - m_bicK;
        EV_DEBUG << "t=" << t.dbl() << " >= k: offs=" << offs << EV_ENDL;
    }

    /* Constant value taken from Experimental Evaluation of Cubic Tcp, available at
     * eprints.nuim.ie/1716/1/Hamiltonpfldnet2007_cubic_final.pdf */
    delta = m_c * std::pow(offs, 3);

    EV_DEBUG << "delta: " << delta << EV_ENDL;

    if (t.dbl() < m_bicK) {
        // below origin
        bicTarget = m_bicOriginPoint - delta;
        EV_DEBUG << "t < k: Bic Target: " << bicTarget << EV_ENDL;
    }
    else {
        // above origin
        bicTarget = m_bicOriginPoint + delta;
        EV_DEBUG << "t >= k: Bic Target: " << bicTarget << EV_ENDL;
    }

    // Next the window target is converted into a cnt or count value. CUBIC will
    // wait until enough new ACKs have arrived that a counter meets or exceeds
    // this cnt value. This is how the CUBIC implementation simulates growing
    // cwnd by values other than 1 segment size.
    if (bicTarget > segCwnd) {
        cnt = segCwnd / (bicTarget - segCwnd);
        EV_DEBUG << "target>cwnd. cnt=" << cnt << EV_ENDL;
    }
    else
        cnt = 100 * segCwnd;

    if (m_lastMaxCwnd == 0 && cnt > m_cntClamp)
        cnt = m_cntClamp;

    // The maximum rate of cwnd increase CUBIC allows is 1 packet per
    // 2 packets ACKed, meaning cwnd grows at 1.5x per RTT.
    return std::max(cnt, 2U);
}

void TcpCubic::pktsAcked(uint32_t segmentsAcked, const simtime_t& rtt)
{
    /* Discard delay samples right after fast recovery */
    if (m_epochStart != -1 && (simTime() - m_epochStart) < m_cubicDelta)
        return;

    /* first time call or link delay decreases */
    if (m_delayMin == -1 || m_delayMin > rtt)
        m_delayMin = rtt;

    /* hystart triggers when cwnd is larger than some threshold */
    if (m_hystart && state->snd_cwnd <= state->ssthresh &&
        state->snd_cwnd >= m_hystartLowWindow * state->snd_effmss)
    {
        hystartUpdate(rtt);
    }
}

void TcpCubic::hystartUpdate(const simtime_t& delay)
{
    if (!m_found) {
        simtime_t now = simTime();

        /* first detection parameter - ack-train detection */
        if ((now - m_lastAck) <= m_hystartAckDelta) {
            m_lastAck = now;

            if ((now - m_roundStart) > m_delayMin) {
                if (m_hystartDetect == HybridSSDetectionMode::PACKET_TRAIN ||
                    m_hystartDetect == HybridSSDetectionMode::BOTH)
                {
                    m_found = true;
                }
            }
        }

        /* obtain the minimum delay of more than sampling packets */
        if (m_sampleCnt < m_hystartMinSamples) {
            if (m_currRtt == -1 || m_currRtt > delay)
                m_currRtt = delay;

            ++m_sampleCnt;
        }
        else if (m_currRtt > m_delayMin + hystartDelayThresh(m_delayMin)) {
            if (m_hystartDetect == HybridSSDetectionMode::DELAY ||
                m_hystartDetect == HybridSSDetectionMode::BOTH)
            {
                m_found = true;
            }
        }

        /*
         * Either one of two conditions are met,
         * we exit from slow start immediately.
         */
        if (m_found) {
            EV_DEBUG << "Exit from SS, immediately :-)" << EV_ENDL;
            state->ssthresh = state->snd_cwnd;
        }
    }
}

simtime_t TcpCubic::hystartDelayThresh(const simtime_t& t) const
{
    simtime_t ret = t;
    if (t > m_hystartDelayMax)
        ret = m_hystartDelayMax;
    else if (t < m_hystartDelayMin)
        ret = m_hystartDelayMin;
    return ret;
}

uint32_t TcpCubic::calculateSsthresh(uint32_t bytesInFlight)
{
    uint32_t segCwnd = state->snd_cwnd / state->snd_effmss;
    EV_DEBUG << "Loss at cWnd=" << segCwnd << " segments in flight=" << bytesInFlight / state->snd_effmss << EV_ENDL;

    /* Wmax and fast convergence */
    if (segCwnd < m_lastMaxCwnd && m_fastConvergence)
        m_lastMaxCwnd = (segCwnd * (1 + m_beta)) / 2; // Section 4.6 in RFC 8312
    else
        m_lastMaxCwnd = segCwnd;

    m_epochStart = -1; // end of epoch

    /* Formula taken from the Linux kernel */
    uint32_t ssThresh = std::max(static_cast<uint32_t>(segCwnd * m_beta), 2U) * state->snd_effmss;

    EV_DEBUG << "SsThresh = " << ssThresh << EV_ENDL;

    return ssThresh;
}

void TcpCubic::cubicReset()
{
    m_lastMaxCwnd = 0;
    m_bicOriginPoint = 0;
    m_bicK = 0;
    m_delayMin = -1;
    m_found = false;
}

} // namespace tcp
} // namespace inet
