//
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


#include <omnetpp.h>
#include "REDQueue.h"


Define_Module(REDQueue);

void REDQueue::initialize()
{
    PassiveQueueBase::initialize();
    queue.setName("l2queue");

    avgQlenVec.setName("avg queue length");
    qlenVec.setName("queue length");
    dropVec.setName("drops");

    // configuration
    wq = par("wq");
    minth = par("minth");
    maxth = par("maxth");
    maxp = par("maxp");
    pkrate = par("pkrate");

    outGate = gate("out");

    // state
    avg = 0;
    q_time = 0;
    count = -1;
    numEarlyDrops = 0;

    WATCH(avg);
    WATCH(q_time);
    WATCH(count);
    WATCH(numEarlyDrops);
}

bool REDQueue::enqueue(cMessage *msg)
{
    //"
    // for each packet arrival
    //    calculate the new average queue size avg:
    //        if the queue is nonempty
    //            avg <- (1-wq)*avg + wq*q
    //        else
    //            m <- f(time-q_time)
    //            avg <- (1-wq)^m * avg
    //"
    if (!queue.empty())
    {
        avg = (1-wq)*avg + wq*queue.length();
    }
    else
    {
        // Note: f() is supposed to estimate the number of packets
        // that could have arrived during the idle interval (see Section 11
        // of the paper). We use pkrate for this purpose.
        double m = SIMTIME_DBL(simTime()-q_time) * pkrate;
        avg = pow(1-wq, m) * avg;
    }

    // statistics
    avgQlenVec.record(avg);

    //"
    //    if minth <= avg < maxth
    //        increment count
    //        calculate probability pa:
    //            pb <- maxp*(avg-minth) / (maxth-minth)
    //            pa <- pb / (1-count*pb)
    //        with probability pa:
    //            mark the arriving packet
    //            count <- 0
    //    else if maxth <= avg
    //        mark the arriving packet
    //        count <- 0
    //    else count <- -1
    //"

    bool mark = false;
    if (minth<=avg && avg<maxth)
    {
        count++;
        double pb = maxp*(avg-minth) / (maxth-minth);
        double pa = pb / (1-count*pb);
        if (dblrand() < pa)
        {
            EV << "Random early packet drop (avg queue len=" << avg << ", pa=" << pa << ")\n";
            mark = true;
            count = 0;
            numEarlyDrops++;
        }
    }
    else if (maxth <= avg)
    {
        EV << "Avg queue len " << avg << " >= maxth, dropping packet.\n";
        mark = true;
        count = 0;
    }
    else
    {
        count = -1;
    }

    // carry out decision
    if (mark || queue.length()>=maxth) // maxth is also the "hard" limit
    {
        delete msg;
        dropVec.record(1);
        return true;
    }
    else
    {
        queue.insert(msg);
        qlenVec.record(queue.length());
        return false;
    }
}

cMessage *REDQueue::dequeue()
{
    if (queue.empty())
        return NULL;

    //"
    // when queue becomes empty
    //    q_time <- time
    //"
    cMessage *pk = (cMessage *)queue.pop();
    if (queue.length()==0)
        q_time = simTime();

    // statistics
    qlenVec.record(queue.length());

    return pk;
}

void REDQueue::sendOut(cMessage *msg)
{
    send(msg, outGate);
}

void REDQueue::finish()
{
    PassiveQueueBase::finish();
    recordScalar("packets dropped early by RED", numEarlyDrops);
}
