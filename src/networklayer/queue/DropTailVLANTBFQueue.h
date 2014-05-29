//
// Copyright (C) 2012 Kyeong Soo (Joseph) Kim
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


#ifndef __INET_DROPTAILVLANTBFQUEUE_H
#define __INET_DROPTAILVLANTBFQUEUE_H

#include <omnetpp.h>
#include <algorithm>
#include <sstream>
#include <vector>
#include "PassiveQueueBase.h"
#include "IQoSClassifier.h"

/**
 * Drop-tail queue with VLAN classifier and token bucket filter (TBF) traffic shaper.
 * See NED for more info.
 */
class INET_API DropTailVLANTBFQueue : public PassiveQueueBase
{
    // type definitions for member variables
    typedef std::vector<bool> BoolVector;
    typedef std::vector<double> DoubleVector;
    typedef std::vector<int> IntVector;
    typedef std::vector<unsigned long> UnsignedLongVector;
    typedef std::vector<unsigned long long> UnsignedLongLongVector;
    typedef std::vector<cMessage *> MsgVector;
    typedef std::vector<cQueue *> QueueVector;
    typedef std::vector<simtime_t> TimeVector;

  protected:
    // configuration
    int fifoCapacity;
    int frameCapacity;
    int numQueues;
    unsigned long long bucketSize;    // in bit; note that the corresponding parameter in NED/INI is in byte.
    double meanRate;
    int mtu;   // in bit; note that the corresponding parameter in NED/INI is in byte.
    double peakRate;

    // state
    cQueue fifo;
    IQoSClassifier *classifier;
    QueueVector queues;
    UnsignedLongLongVector meanBucketLength;  // vector of the number of tokens (bits) in the bucket for mean rate/burst control
    IntVector peakBucketLength;  // vector of the number of tokens (bits) in the bucket for peak rate/MTU control
    TimeVector lastTime; // vector of the last time the TBF used
    BoolVector conformityFlag;  // vector of flag to indicate whether the HOL frame conforms to TBF

    /* // state: RR scheduler */
    /* int currentQueueIndex;  // index of a queue whose HOL frame is scheduled for TX during the last RR scheduling */

    // statistics
    bool warmupFinished;        ///< if true, start statistics gathering
    DoubleVector numBitsSent;
    UnsignedLongVector numPktsReceived; // redefined from PassiveQueueBase with 'name hiding'
    UnsignedLongVector numPktsDropped;  // redefined from PassiveQueueBase with 'name hiding'
    UnsignedLongVector numPktsUnshaped;
    UnsignedLongVector numPktsSent;

    // timer
    MsgVector conformityTimer;  // vector of timer indicating that enough tokens will be available for the transmission of the HOL frame

    cGate *outGate;

  public:
    DropTailVLANTBFQueue();
    virtual ~DropTailVLANTBFQueue();

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
     * Redefined from PassiveQueueBase.
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
    virtual bool voqEnqueue(cMessage *msg);

    virtual cMessage *voqDequeue(int flowIndex);

    virtual bool isConformed(int flowIndex, int pktLength);

    virtual void updateTbf(int flowIndex, int pktLength);
    
    virtual void triggerConformityTimer(int flowIndex);

    virtual void dumpTbfStatus(int flowIndex);
};

#endif


