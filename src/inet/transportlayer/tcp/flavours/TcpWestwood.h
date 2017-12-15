//
// Copyright (C) 2013 Maria Fernandez, Carlos Calafate, Juan-Carlos Cano and
// Pietro Manzoni
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_TCPWESTWOOD_H
#define __INET_TCPWESTWOOD_H

#include "inet/common/INETDefs.h"

#include "inet/transportlayer/tcp/flavours/TcpBaseAlg.h"
#include "inet/transportlayer/tcp/flavours/TcpSegmentTransmitInfoList.h"

namespace inet {

namespace tcp {

/**
 * State variables for TcpWestwood.
 */
class INET_API TcpWestwoodStateVariables : public TcpBaseAlgStateVariables
{
  public:
    TcpWestwoodStateVariables();
    ~TcpWestwoodStateVariables();
    virtual std::string str() const override;
    virtual std::string detailedInfo() const override;

    uint32 ssthresh;    ///< slow start threshold

    simtime_t w_RTTmin;    // min RTT
    double w_a;    // threshold reduction factor for ssthresh calculation

    simtime_t w_lastAckTime;    // last received ack time

    double w_bwe;
    double w_sample_bwe;

    TcpSegmentTransmitInfoList regions;
};

class INET_API TcpWestwood : public TcpBaseAlg
{
  protected:
    TcpWestwoodStateVariables *& state;    // alias to TCLAlgorithm's 'state'

    /** Create and return a TCPvegasStateVariables object. */
    virtual TcpStateVariables *createStateVariables() override
    {
        return new TcpWestwoodStateVariables();
    }

    /** Utility function to recalculate ssthresh */
    virtual void recalculateSlowStartThreshold();

    /** Redefine what should happen on retransmission */
    virtual void processRexmitTimer(TcpEventCode& event) override;

    /** Recalculate BWE */
    virtual void recalculateBWE(uint32 cumul_ack);

  public:
    /** Ctor */
    TcpWestwood();

    /** Redefine what should happen when data got acked, to add congestion window management */
    virtual void receivedDataAck(uint32 firstSeqAcked) override;

    /** Redefine what should happen when dupAck was received, to add congestion window management */
    virtual void receivedDuplicateAck() override;

    /** Called after we send data */
    virtual void dataSent(uint32 fromseq) override;

    virtual void segmentRetransmitted(uint32 fromseq, uint32 toseq) override;
};

} // namespace tcp

} // namespace inet

#endif // ifndef __INET_TCPWESTWOOD_H

