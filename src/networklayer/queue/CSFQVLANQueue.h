//
// Copyright (C) 2013 Kyeong Soo (Joseph) Kim
// Copyright (C) 2005 Andras Varga
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


#ifndef __INET_CSFQVLANQUEUE_H
#define __INET_CSFQVLANQUEUE_H

#include <omnetpp.h>
//#include <algorithm>
#include <sstream>
#include <vector>
#include "PassiveQueueBase.h"
#include "BasicTokenBucketMeter.h"
#include "IQoSClassifier.h"

// define constants
#define KALPHA 29 // 0.99^29 = 0.75
//#define CSFQ_LOG
//#define PENALTY_BOX  /* use penalty box in conjucntion with CSFQ */

#ifdef PENALTY_BOX
#define MONITOR_TABLE_SIZE 10
#define PUNISH_TABLE_SIZE  10
#define DROPPED_ARRAY_SIZE 100
#define PUNISH_THRESH  1.3  /*
                             * when the flow's rate exceeds
                             * PUNISH_THRESH times the fair rate
                             * the flow is punished
                             */
#define GOOD_THRESH 0.7     /*
                             * when the punished flow's rate is
                             * GOOD_THRESH times smaller than the
                             * link's fair rate the flow
                             * is no longer punished
                             */
#define BETA 0.98            /*
                              * (1 - BETA) = precentage by which
                              * link's fair rate is decreased when a
                              * forced drop occurs
                              */

typedef struct identHash {
    int valid_;
    double prevTime_;
    double estNormRate_;
} IdentHashTable;
#endif

//#define SRCID       /* identify flow based of source id rather than flow id */
//#define CSFQ_RLMEX  /* used for RLM experiments only */


/**
 * Incoming packets are classified by an external VLAN classifier and
 * metered by an external token bucket meters before being put into
 * a common FIFO queue, non-conformant ones being processed core-stateless
 * fair queueing (CSFQ) algorithm for proportional allocation of excess
 * bandwidth.
 * See NED for more info.
 */
class INET_API CSFQVLANQueue : public PassiveQueueBase
{
    typedef struct
    {
        double alpha;       // output link fair rate
        double K_alpha;     // averaging interval for rate estimation
        simtime_t lastArv;  // the arrival time of the last packet (in sec)
        simtime_t startTime;    // used to store the start of an interval of length K_alpha
        double linkRate;    // link rate (in bps)
        double rateTotal;   // aggregate arrival rate (i.e., \hat{A})
        double rateAlpha;   // aggregate forwarded rate corresponding to crt. alpha (i.e., \hat{F})
        double tmpAlpha;    // used to compute the largest label of a packet, i.e., the largest flow rate seen during an interval of length K_alpha
        bool congested;     // indicate whether the link is congested or not
        int kalpha;        // maximum number of times the fair rate (alpha) can be decreased when the queue overflows, during a time interval of length K_alpha
        int pktLength;      // the total number of bits enqueued between two consecutive rate estimations. usually, this represent the size of one packet;
                            // however, if more packets are received at the same time this will represent the cumulative size of all packets received simultaneously
        int pktLengthEnqueued;    // same as above, but for the total number of bytes that are enqueued between consecutive rate estimations
    } CSFQState;

    typedef struct
    {
        double weight;  // flow weight (set to TB mean rate)
        double K;   // averaging interval for rate estimation
        double estRate[2];      // estimated rates; 0 for conformed and 1 for non-conformed packets
        simtime_t prevTime[2];  // time of previously arrived packet; 0 for conformed and 1 for non-conformed packet
        // internal statistics
        int sumPktSize;// keep track of packets that arrive at the same time
        int numArv_;// number of arrived packets
        int numDpt_;// number of dropped packets
        int numDropped_;// number of dropped packets
    } FlowState;

    // type definitions for member variables
    typedef std::vector<bool> BoolVector;
    typedef std::vector<double> DoubleVector;
    typedef std::vector<FlowState> FlowStateVector;
    typedef std::vector<int> IntVector;
    typedef std::vector<long long> LongLongVector;
    typedef std::vector<BasicTokenBucketMeter *> TbmVector;

  protected:
    // general
    int numFlows;

    // VLAN classifier
    IQoSClassifier *classifier;

    // FIFO
    cQueue fifo;
    int queueSize;
    int currentQueueSize;   // in bit
    int queueThreshold;

    // token bucket meters
    TbmVector tbm;

    // CSFQ
    CSFQState csfq;    // CSFQ-related states
    FlowStateVector flowState;  // vector of flowState

    // statistics
    bool warmupFinished;        ///< if true, start statistics gathering
    DoubleVector numBitsSent;
    IntVector numPktsReceived; // redefined from PassiveQueueBase with 'name hiding'
    IntVector numPktsDropped;  // redefined from PassiveQueueBase with 'name hiding'
    IntVector numPktsConformed;
    IntVector numPktsSent;

    cGate *outGate;

    // debugging
#ifdef NDEBUG
    // do nothing for release mode
#else
    typedef std::vector<cOutVector *> OutVectorVector;
    std::vector<OutVectorVector> estRateVectors;
#endif

  public:
    CSFQVLANQueue();
    virtual ~CSFQVLANQueue();

  protected:
    virtual void initialize();

    /**
     * Redefined from PassiveQueueBase.
     */
    virtual void handleMessage(cMessage *msg);

    /**
     * Redefined from PassiveQueueBase.
     */
    virtual void finish();

    /**
     * Redefined from PassiveQueueBase.
     */
    virtual bool enqueue(cMessage *msg);

    /**
     * Redefined from PassiveQueueBase to implement round-robin (RR) scheduling.
     */
    virtual cMessage *dequeue();

    /**
     * Redefined from PassiveQueueBase.
     */
    virtual void sendOut(cMessage *msg);

    /**
     * The queue should send a packet whenever this method is invoked.
     * If the queue is currently empty, it should send a packet when
     * when one becomes available.
     */
    virtual void requestPacket();

    /**
     * For CSFQ.
     */
    virtual void estimateAlpha(int pktLength, double arrvRate, simtime_t arrvTime, int dropped);
    virtual double estimateRate(int flowId, int pktLength, simtime_t arrvTime, int color);
};

#endif


