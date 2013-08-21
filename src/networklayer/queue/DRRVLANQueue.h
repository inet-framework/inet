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


#ifndef __INET_DRRVLANQUEUE_H
#define __INET_DRRVLANQUEUE_H

#include <algorithm>
#include <list>
#include <omnetpp.h>
#include <sstream>
#include <stdlib.h>     // atoi()
#include <vector>
#include "PassiveQueueBase.h"
#include "BasicTokenBucketMeter.h"
#include "IQoSClassifier.h"

/**
 * Priority queueing (PQ) over FIFO queue and per-flow queues with
 * deficit round-robin (DRR) scheduler.
 * Incoming packets are classified by an external VLAN classifier and
 * metered by an external token bucket meters before being put into
 * a FIFO or per-flow queues.
 * See NED for more info.
 */
class INET_API DRRVLANQueue : public PassiveQueueBase
{
  protected:
    // type definitions for member variables
    typedef std::list<int> IntList;
    typedef std::vector<bool> BoolVector;
    typedef std::vector<double> DoubleVector;
    typedef std::vector<int> IntVector;
    typedef std::vector<long long> LongLongVector;
    typedef std::vector<BasicTokenBucketMeter *> TbmVector;
    typedef std::vector<cMessage *> MsgVector;
    typedef std::vector<cQueue *> QueueVector;
    typedef std::vector<simtime_t> TimeVector;

  protected:
    // general
    int numFlows;

    // VLAN classifier
    IQoSClassifier *classifier;

    // token bucket meters
    TbmVector tbm;

    // FIFO
    cQueue fifo;
    int fifoSize;           // in byte
    int fifoCurrentSize;    // in byte

    // DRR scheduler
//    int currentVoqIndex;        ///< index of a VOQ whose HOL frame is scheduled for TX during the last RR scheduling
    IntList activeList;         ///< list of active queues with frames
    IntVector deficitCounters;  ///< vector of deficit counters in DRR scheduling
    IntVector quanta;           ///< vector of quantum  in DRR scheduling
    QueueVector voq;            ///< per-flow virtual output queues (VOQs)
    int voqSize;                ///< VOQ size in byte
    IntVector voqCurrentSize;   ///< current size of VOQs in byte
    bool continuation;          ///< flag indicating whether the previous run is continued.

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
    cOutVector pktReqVector;
#endif

  public:
    DRRVLANQueue();
    virtual ~DRRVLANQueue();

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
};

#endif
