//
// Copyright (C) 2001-2006  Sergio Andreozzi
// Copyright (C) 2009 A. Ariza Universidad de Malaga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#ifndef __INET_WEIGHTED_FAIR_QUEUE_H
#define __INET_WEIGHTED_FAIR_QUEUE_H

#include <vector>
#include <queue>

#include "INETDefs.h"

#include "PassiveQueueBase.h"
#include "IQoSClassifier.h"


/**
 * Queue module that implements Weighted Fair Queueing (WFQ).
 * See NED file for more info.
 */
class INET_API WeightedFairQueue : public PassiveQueueBase
{
  protected:
    class SubQueueData
    {
      public:
        double finish_t;  // for WFQ
        double queueMaxRate;
        double queueWeight;
        unsigned int B;    // set of active queues in the GPS reference system
        // B[]!=0 queue is active, ==0 queue is inactive

        std::queue<double> time;

        double wq;    // queue weight
        double minth; // minimum threshold for avg queue length
        double maxth; // maximum threshold for avg queue length
        double maxp;  // maximum value for pb
        double pkrate; // number of packets expected to arrive per second (used for f())

        // state (see NED file and paper for meaning of RED variables)
        double avg;       // average queue size
        simtime_t q_time; // start of the queue idle time
        int count;        // packets since last marked packet
        int numEarlyDrops;

        SubQueueData()
        {
            finish_t = 0;
            queueMaxRate = 0;
            queueWeight = 0;
            B = 0;
            avg = 0;
            q_time = 0;
            count = 0;
            numEarlyDrops = 0;
        }

    };

    // statistics
    static simsignal_t queueLengthSignal;
    static simsignal_t earlyDropPkByQueueSignal;

  protected:
    int frameCapacity;
    std::vector<SubQueueData> subqueueData;
    std::vector<cQueue> queueArray;
    int numQueues;
    IQoSClassifier *classifier;
    cGate *outGate;
    bool useRED;
    long totalLength;

    double bandwidth; // total link bandwidth
    double  virt_time, last_vt_update, sum;
    bool GPS_idle;
    double safe_limit;

  public:
    WeightedFairQueue();
    ~WeightedFairQueue();

    virtual void setBandwidth(double val) { bandwidth = val; }

    virtual double getBandwidth() const { return bandwidth; }

    virtual void setQueueWeight(int i, double val)
    {
        if (i < 0 || i >= numQueues)
            opp_error("Queue index out of range");
        subqueueData[i].queueWeight = val;
    }

    virtual double getQueueWeight(int i) const
    {
        if (i < 0 || i >= numQueues)
            opp_error("Queue index out of range");
        return subqueueData[i].queueWeight;
    }

  protected:
    virtual void initialize();

    virtual bool REDTest(cMessage *msg, int queueIndex);

    // methods redefined from PassiveQueueBase:
    virtual cMessage *enqueue(cMessage *msg);
    virtual cMessage *dequeue();
    virtual void sendOut(cMessage *msg);
    virtual bool isEmpty();
};

#endif

