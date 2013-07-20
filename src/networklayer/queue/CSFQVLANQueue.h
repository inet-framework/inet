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
#include <algorithm>
#include <sstream>
#include <vector>
#include "PassiveQueueBase.h"
#include "BasicTokenBucketMeter.h"
#include "IQoSClassifier.h"

// define constants
#define KALPHA 29 // 0.99^29 = 0.75

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
    // type definitions for member variables
    typedef std::vector<bool> BoolVector;
    typedef std::vector<double> DoubleVector;
    typedef std::vector<int> IntVector;
    typedef std::vector<long long> LongLongVector;
    typedef std::vector<BasicTokenBucketMeter *> TbmVector;
    typedef std::vector<simtime_t> TimeVector;

  protected:
    // general
    int numFlows;
    double linkRate;    // link rate (in bps)

    // VLAN classifier
    IQoSClassifier *classifier;

    // FIFO
    cQueue fifo;
    int queueSize;          // in byte
    int currentQueueSize;   // in byte
    int queueThreshold;     // in byte

    // token bucket meters
    TbmVector tbm;

    // CSFQ++: System-wide variables
    double K;               // averaging interval for flow rate estimation
    double K_alpha;         // averaging interval for overall rate estimation
    double excessBW;        // excess bandwidth (i.e., C_{ex})
    double fairShareRate;   // normalized fair share rate for non-conformed flows
    double rateTotal;       // aggregate arrival rate of non-conformed packets (i.e., \hat{A})
    double rateEnqueued;    // aggregate enqueued rate of non-conformed packets (i.e., \hat{F})
    double maxRate;         // used to compute the largest normalized flow rate seen during the interval of K_alpha
    simtime_t lastArv;      // the arrival time of the last packet (in sec)
    simtime_t startTime;    // used to store the start of an interval of length K_alpha
    int kalpha;             // maximum number of times the fair rate (alpha) can be decreased when the queue overflows, during a time interval of length K_alpha
    int sumBitsTotal;       // the total number of bits enqueued between two consecutive rate estimations. usually, this represent the size of one packet;
                            // however, if more packets are received at the same time this will represent the cumulative size of all packets received simultaneously
    int sumBitsEnqueued;    // same as above, but for the total number of bytes that are enqueued between consecutive rate estimations
    bool congested;         // flag indicating whether the link is congested or not

    // CSFQ++: Flow-specific variables
    DoubleVector weight;    // vector of flow weights (set to TB mean rate)
    IntVector sumBits;      // keep track of packets that arrive at the same time
    std::vector< std::vector<double> > flowRate;    // 2-dimensional vector of estimated flow rates;
                                                    // - second index of 0 for conformed and 1 for non-conformed packets
    std::vector< std::vector<simtime_t> > prevTime; // 2-dimensional vector of time of previously arrived packet;
                                                    // - second index of 0 for conformed and 1 for non-conformed packet

    // statistics
    bool warmupFinished;        ///< if true, start statistics gathering
    DoubleVector numBitsSent;
    IntVector numPktsReceived; // redefined from PassiveQueueBase with 'name hiding'
    IntVector numPktsDropped;  // redefined from PassiveQueueBase with 'name hiding'
    IntVector numPktsConformed;
    IntVector numPktsSent;

    cGate *outGate;

    // debugging
#ifndef NDEBUG
    cOutVector excessBWVector;
    cOutVector fairShareRateVector;
    cOutVector pktReqVector;
    cOutVector queueLengthVector;
    cOutVector queueSizeVector;
    cOutVector rateTotalVector;
    cOutVector rateEnqueuedVector;
    typedef std::vector<cOutVector *> OutVectorVector;
    std::vector<OutVectorVector> estRateVectors;
#endif

  public:
    CSFQVLANQueue();
    virtual ~CSFQVLANQueue();

  protected:
//    virtual void initialize();
    virtual void initialize(int stage);
    virtual int numInitStages() const {return 2;}

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
    virtual void estimateAlpha(int pktLength, double rate, simtime_t arrvTime, int dropped);
    virtual double estimateRate(int flowId, int pktLength, simtime_t arrvTime, int color);
};

#endif


