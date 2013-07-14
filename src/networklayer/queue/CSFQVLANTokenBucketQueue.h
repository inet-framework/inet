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


#ifndef __INET_CSFQVLANTOKENBUCKETQUEUE_H
#define __INET_CSFQVLANTOKENBUCKETQUEUE_H

#include <omnetpp.h>
#include <algorithm>
#include <sstream>
#include <vector>
#include "PassiveQueueBase.h"
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
#define GOOD_THRESH 0.7  /*
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
 * Drop-tail queue with VLAN classifier, token bucket filter (TBF) traffic shaper,
 * and round-robin (RR) scheduler.
 * See NED for more info.
 */
class INET_API CSFQVLANTokenBucketQueue : public PassiveQueueBase
{
    typedef struct
    {
        double alpha_;
        double kLink_;
        double lastArv_;
        double lArv_;
        double rate_;
        double rateAlpha_;
        double rateTotal_;
        double tmpAlpha_;
        int congested_;
        int kalpha_;
        int pktLength_;
        int pktLengthE_;
    } CSFQState;

    typedef struct
    {
        double weight_;     // flow weight
        double k_;// averaging interval for rate estimation
        double estRate_;// estimated rate
        double prevTime_;
        // internal statistics
        int size_;// keep track of packets that arrive at the same time
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
    typedef std::vector<cMessage *> MsgVector;
    typedef std::vector<cQueue *> QueueVector;
    typedef std::vector<simtime_t> TimeVector;

  protected:
    // configuration
    int numFlows;
    int queueSize;
    int queueThreshold;
    LongLongVector bucketSize;  // in bit; note that the corresponding parameter in NED/INI is in byte.
    DoubleVector meanRate;  // in bps
    IntVector mtu;   // in bit; note that the corresponding parameter in NED/INI is in byte.
    DoubleVector peakRate;  // in bps

    // VLAN classifier
    IQoSClassifier *classifier;

    // FIFO
    cQueue fifo;
    int currentQueueSize;   // in bit

    // TBF
    LongLongVector meanBucketLength;  // vector of the number of tokens (bits) in the bucket for mean rate/burst control
    IntVector peakBucketLength;  // vector of the number of tokens (bits) in the bucket for peak rate/MTU control
    TimeVector lastTime; // vector of the last time the TBF used
    BoolVector conformityFlag;  // vector of flag to indicate whether the HOL frame conforms to TBF

    // CSFQ
    CSFQState csfq;    // CSFQ-related states
    FlowStateVector flowState;  // vector of flowState
    DoubleVector conformedRate; // vector of estimated rate of conformed flow
    DoubleVector nonconformedRate;  // vector of estimated rate of nonconformed flow

    // statistics
    bool warmupFinished;        ///< if true, start statistics gathering
    DoubleVector numBitsSent;
    IntVector numPktsReceived;
    IntVector numPktsDropped;
    IntVector numPktsUnshaped;
    IntVector numPktsSent;

    cGate *outGate;

  public:
    CSFQVLANTokenBucketQueue();
    virtual ~CSFQVLANTokenBucketQueue();

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
     * Newly defined.
     */
    virtual bool isConformed(int queueIndex, int pktLength);
//    virtual void triggerConformityTimer(int queueIndex, int pktLength);
    virtual void dumpTbfStatus(int queueIndex);

    /**
     * From CSFQ.
     */
    virtual void estimateAlpha(int pktLength, double arrvRate, double arrvTime, int enqueue);
    virtual double estimateRate(int flowId, int pktLength, double arrvTime);
};

#endif


