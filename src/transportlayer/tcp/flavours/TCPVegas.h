#ifndef __INET_TCPVEGAS_H
#define __INET_TCPVEGAS_H

#include "inet/common/INETDefs.h"

#include "inet/transportlayer/tcp/flavours/TCPBaseAlg.h"
#include "inet/transportlayer/tcp/flavours/TCPSegmentTransmitInfoList.h"

namespace inet {

namespace tcp {

/**
 * State variables for TCPVegas.
 */
class INET_API TCPVegasStateVariables : public TCPBaseAlgStateVariables
{
  public:
    TCPVegasStateVariables();
    ~TCPVegasStateVariables();
    virtual std::string info() const;
    virtual std::string detailedInfo() const;

    uint32 v_recoverypoint;
    simtime_t v_cwnd_changed;    // last time cwnd changes because of a rtx.

    simtime_t v_baseRTT;
    simtime_t v_sumRTT;    // sum of rtt's measured within one RTT
    int v_cntRTT;    // # of rtt's measured within one RTT
    uint32 v_begseq;    // register next pkt to be sent,for rtt calculation in receivedDataAck
    simtime_t v_begtime;    // register time for rtt calculation

    simtime_t v_rtt_timeout;    // vegas fine-grained timeout
    simtime_t v_sa;    // average for vegas fine-grained timeout
    simtime_t v_sd;    // deviation for vegas fine-grained timeout

    TCPSegmentTransmitInfoList regions;

    uint32 ssthresh;    ///< slow start threshold

    bool v_inc_flag;    // for slow start: "exponential growth only every other RTT"
    bool v_incr_ss;    // to control no incr. cwnd if during slowstart ssthresh has been exceeded before the rtt is over
    int32 v_incr;    // incr/decr
    uint32 v_worried;    // pkts a to retransmit due to vegas fine-grained timeout
};

class INET_API TCPVegas : public TCPBaseAlg
{
  protected:
    TCPVegasStateVariables *& state;    // alias to TCPAlgorithm's 'state'

    /** Create and return a TCPvegasStateVariables object. */
    virtual TCPStateVariables *createStateVariables()
    {
        return new TCPVegasStateVariables();
    }

    /** Utility function to recalculate ssthresh */
    virtual void recalculateSlowStartThreshold();

    /** Redefine what should happen on retransmission */
    virtual void processRexmitTimer(TCPEventCode& event);

  public:
    /** Ctor */
    TCPVegas();

    /** Redefine what should happen when data got acked, to add congestion window management */
    virtual void receivedDataAck(uint32 firstSeqAcked);

    /** Redefine what should happen when dupAck was received, to add congestion window management */
    virtual void receivedDuplicateAck();

    /** Called after we send data */
    virtual void dataSent(uint32 fromseq);

    virtual void segmentRetransmitted(uint32 fromseq, uint32 toseq);
};

} // namespace tcp

} // namespace inet

#endif // ifndef __INET_TCPVEGAS_H

