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

#ifndef __INET_TCPCUBIC_H
#define __INET_TCPCUBIC_H

#include "inet/transportlayer/tcp/flavours/TcpAlgorithmBase.h"
#include "inet/transportlayer/tcp/flavours/TcpClassicAlgorithmBaseState_m.h"

namespace inet {
namespace tcp {

/**
 * \brief The Cubic Congestion Control Algorithm
 *
 * TCP Cubic is a protocol that enhances the fairness property
 * of Bic while retaining its scalability and stability. The main feature is
 * that the window growth function is defined in real time in order to be independent
 * from the RTT. More specifically, the congestion window of Cubic is determined
 * by a function of the elapsed time from the last window reduction.
 *
 * The Cubic code is quite similar to that of Bic.
 * The main difference is located in the method Update, an edit
 * necessary for satisfying the Cubic window growth, that can be tuned with
 * the attribute C (the Cubic scaling factor).
 *
 * Following the Linux implementation, we included the Hybrid Slow Start,
 * that effectively prevents the overshooting of slow start
 * while maintaining a full utilization of the network. This new type of slow
 * start can be disabled through the HyStart attribute.
 *
 * CUBIC TCP is implemented and used by default in Linux kernels 2.6.19
 * and above; this version follows the implementation in Linux 3.14, which
 * slightly differ from the CUBIC paper. It also features HyStart.
 *
 * Home page:
 *      https://web.archive.org/web/20080607093013/http://netsrv.csc.ncsu.edu/twiki/bin/view/Main/BIC
 * The work starts from the implementation of CUBIC TCP in
 * Sangtae Ha, Injong Rhee and Lisong Xu,
 * "CUBIC: A New TCP-Friendly High-Speed TCP Variant"
 * in ACM SIGOPS Operating System Review, July 2008.
 * Available from:
 *  https://web.archive.org/web/20160505194415/http://netsrv.csc.ncsu.edu/export/cubic_a_new_tcp_2008.pdf
 *
 * CUBIC integrates a new slow start algorithm, called HyStart.
 * The details of HyStart are presented in
 *  Sangtae Ha and Injong Rhee,
 *  "Taming the Elephants: New TCP Slow Start", NCSU TechReport 2008.
 * Available from:
 *  https://web.archive.org/web/20160528233754/http://netsrv.csc.ncsu.edu/export/hystart_techreport_2008.pdf
 *
 * More information on this implementation: http://dl.acm.org/citation.cfm?id=2756518
 */
class INET_API TcpCubic : public TcpAlgorithmBase
{
  public:
    /**
     * \brief Values to detect the Slow Start mode of HyStart
     */
    enum HybridSSDetectionMode
    {
        PACKET_TRAIN = 1, //!< Detection by trains of packet
        DELAY = 2,        //!< Detection by delay value
        BOTH = 3,         //!< Detection by both
    };

    TcpCubic();

    virtual void initialize() override;
    virtual void established(bool active) override;
    virtual void processRexmitTimer(TcpEventCode& event) override;
    virtual void receivedAckForAlreadyAckedData(const TcpHeader *tcpHeader, uint32_t payloadLength) override;
    virtual void receivedAckForUnackedData(uint32_t firstSeqAcked) override;
    virtual void receivedAckForUnsentData(uint32_t seq) override;
    virtual void receivedDuplicateAck() override;
    virtual uint32_t getBytesInFlight() const override;

    uint32_t calculateSsthresh(uint32_t bytesInFlight);

  protected:
    TcpClassicAlgorithmBaseStateVariables *& state; // alias to TcpAlgorithm's 'state'
    ITcpRecovery *recovery = nullptr;

    bool m_fastConvergence; //!< Enable or disable fast convergence algorithm
    double m_beta;          //!< Beta for cubic multiplicative increase

    bool m_hystart;                        //!< Enable or disable HyStart algorithm
    HybridSSDetectionMode m_hystartDetect; //!< Detect way for HyStart algorithm
    uint32_t m_hystartLowWindow;           //!< Lower bound cWnd for hybrid slow start (segments)
    simtime_t m_hystartAckDelta;                //!< Spacing between ack's indicating train
    simtime_t m_hystartDelayMin;                //!< Minimum time for hystart algorithm
    simtime_t m_hystartDelayMax;                //!< Maximum time for hystart algorithm
    uint8_t m_hystartMinSamples; //!< Number of delay samples for detecting the increase of delay

    uint32_t m_initialCwnd; //!< Initial cWnd
    uint8_t m_cntClamp;     //!< Modulo of the (avoided) float division for cWnd

    double m_c; //!< Cubic Scaling factor

    // Cubic parameters
    uint32_t m_cWndCnt;        //!<  cWnd integer-to-float counter
    uint32_t m_lastMaxCwnd;    //!<  Last maximum cWnd
    uint32_t m_bicOriginPoint; //!<  Origin point of bic function
    double m_bicK;             //!<  simtime_t to origin point from the beginning
                               //    of the current epoch (in s)
    simtime_t m_delayMin;           //!<  Min delay
    simtime_t m_epochStart;         //!<  Beginning of an epoch
    bool m_found;              //!<  The exit point is found?
    simtime_t m_roundStart;         //!<  Beginning of each round
    uint32_t m_endSeq; //!<  End sequence of the round
    simtime_t m_lastAck;            //!<  Last time when the ACK spacing is close
    simtime_t m_cubicDelta;         //!<  simtime_t to wait after recovery before update
    simtime_t m_currRtt;            //!<  Current Rtt
    uint32_t m_sampleCnt;      //!<  Count of samples for HyStart

  protected:
    virtual TcpStateVariables *createStateVariables() override { return new TcpClassicAlgorithmBaseStateVariables(); }

    void pktsAcked(uint32_t segmentsAcked, const simtime_t& rtt);
    void increaseWindow(uint32_t segmentsAcked);

    /**
     * \brief Reset HyStart parameters
     */
    void hystartReset();

    /**
     * \brief Reset Cubic parameters
     */
    void cubicReset();

    /**
     * \brief Cubic window update after a new ack received
     * \returns the congestion window update counter
     */
    uint32_t update();

    /**
     * \brief Update HyStart parameters
     *
     * \param delay Delay for HyStart algorithm
     */
    void hystartUpdate(const simtime_t& delay);

    /**
     * \brief Clamp time value in a range
     *
     * The returned value is t, clamped in a range specified
     * by attributes (HystartDelayMin < t < HystartDelayMax)
     *
     * \param t simtime_t value to clamp
     * \return t itself if it is in range, otherwise the min or max
     * value
     */
    simtime_t hystartDelayThresh(const simtime_t& t) const;
};

} // namespace tcp
} // namespace inet

#endif // __INET_TCPCUBIC_H
