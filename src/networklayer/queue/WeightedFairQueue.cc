//
// Copyright (C) 2001-2006  Sergio Andreozzi
// Copyright (C) 2009 A. Ariza Universidad de M�laga
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

/***************************************************************************/

#include "WeightedFairQueue.h"
Define_Module(WeightedFairQueue);

simsignal_t WeightedFairQueue::queueLengthSignal = SIMSIGNAL_NULL;
simsignal_t WeightedFairQueue::earlyDropPkBytesSignal = SIMSIGNAL_NULL;

void WeightedFairQueue::initialize()
{
    PassiveQueueBase::initialize();
    // configuration
    frameCapacity = par("frameCapacity");
    const char *classifierClass = par("classifierClass");
    bandwidth  = par("Bandwidth");

    queueLengthSignal = registerSignal("queueLength");
    earlyDropPkBytesSignal = registerSignal("earlyDropPkBytes");

    emit(queueLengthSignal, lotalLength);

    classifier = check_and_cast<IQoSClassifier*>(createOne(classifierClass));

    const char *vstr = par("queueWeight").stringValue();
    std::vector<double> queueWeight = cStringTokenizer(vstr).asDoubleVector();
    numQueues = classifier->getNumQueues();
    if (numQueues<(int)queueWeight.size())
        numQueues = queueWeight.size();
    useRed = par("UseRed");
    for (int i=0; i<numQueues; i++)
    {
        SubQueueData queueData;
        char buf[32];
        sprintf(buf, "WFqueue-%d", i);
        cQueue queue(buf);
        subqueueData.push_back(queueData);
        queueArray.push_back(queue);
        subqueueData[i].queueWeight = 1;
        subqueueData[i].wq= par("wq");    // queue weight
        subqueueData[i].minth= par("minth"); // minimum threshold for avg queue length
        subqueueData[i].maxth= par("maxth"); // maximum threshold for avg queue length
        subqueueData[i].maxp=par("maxp");  // maximum value for pb
        subqueueData[i].pkrate= par("pkrate"); // number of packets expected to arrive per second (used for f())
    }

    for (unsigned int i=0; i<queueWeight.size(); i++)
    {
        subqueueData[i].queueWeight = queueWeight[i];
    }
}


bool WeightedFairQueue::RedTest(cMessage *msg,int queueIndex)
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

	if (!useRed)
		return false;


	double *wq= &subqueueData[queueIndex].wq;    // queue weight
	double *minth= &subqueueData[queueIndex].minth; // minimum threshold for avg queue length
	double *maxth= &subqueueData[queueIndex].maxth; // maximum threshold for avg queue length
	double *maxp= &subqueueData[queueIndex].maxp;  // maximum value for pb
	double *pkrate= &subqueueData[queueIndex].pkrate; // number of packets expected to arrive per second (used for f())

	        // state (see NED file and paper for meaning of RED variables)
	double *avg= &subqueueData[queueIndex].avg;       // average queue size
	simtime_t *q_time= &subqueueData[queueIndex].q_time; // start of the queue idle time
	int *count= &subqueueData[queueIndex].count;        // packets since last marked packet
	int *numEarlyDrops= &subqueueData[queueIndex].numEarlyDrops;

    if (!queueArray[queueIndex].empty())
    {
    	(*avg) = (1-(*wq))*(*avg) +(*wq)*queueArray[queueIndex].length();
    }
    else
    {
        // Note: f() is supposed to estimate the number of packets
        // that could have arrived during the idle interval (see Section 11
        // of the paper). We use pkrate for this purpose.
        double m = SIMTIME_DBL(simTime()-(*q_time)) * (*pkrate);
        (*avg) = pow(1-(*wq), m) * (*avg);
    }

    bool mark = false;
    if (*minth<=*avg && *avg<*maxth)
    {
        (*count)++;
        double pb = (*maxp)*((*avg)-(*minth)) / ((*maxth)-(*minth));
        double pa = pb / (1-(*count)*pb);
        if (dblrand() < pa)
        {
            EV << "Random early packet drop (avg queue len=" << avg << ", pa=" << pa << ")\n";
            mark = true;
            (*count) = 0;
            (*numEarlyDrops)++;
        }
    }
    else if (*maxth <= *avg)
    {
        EV << "Avg queue len " << avg << " >= maxth, dropping packet.\n";
        mark = true;
        (*count) = 0;
    }
    else
    {
        (*count) = -1;
    }

    // carry out decision
    if (mark || queueArray[queueIndex].length()>=(*maxth)) // maxth is also the "hard" limit
        return true;
    else
        return false;
}


cMessage * WeightedFairQueue::enqueue(cMessage *msg)
{
    int queueIndex = classifier->classifyPacket(msg);

    if (RedTest(msg,queueIndex))
    {
    	emit(earlyDropPkBytesSignal, (long)(PK(msg)->getByteLength()));
    	return msg;
    }

    if (frameCapacity && queueArray[queueIndex].length() >= frameCapacity)
    {
        EV << "Queue " << queueIndex << " full, dropping packet.\n";
        return msg;
    }
    else
    {
        queueArray[queueIndex].insert(msg);
        if (GPS_idle)
        {
            last_vt_update=SIMTIME_DBL(simTime());
            virt_time=0;
            GPS_idle=false;
        }
        else
        {
            virt_time=virt_time+(SIMTIME_DBL(simTime())-last_vt_update)/sum;
            last_vt_update=SIMTIME_DBL(simTime());
        }
        double pkleng =(double)PK(msg)->getBitLength();
        subqueueData[queueIndex].finish_t = (subqueueData[queueIndex].finish_t > virt_time ?
                                             subqueueData[queueIndex].finish_t:virt_time)+
                                            pkleng /(subqueueData[queueIndex].queueWeight*bandwidth);
        subqueueData[queueIndex].time.push(subqueueData[queueIndex].finish_t);
        if ((subqueueData[queueIndex].B++)==0)
            sum=sum+(double)subqueueData[queueIndex].queueWeight;
        if ( fabs(sum) < safe_limit )
            sum=0;
        lotalLength++;
        emit(queueLengthSignal, lotalLength);

        return false;
    }
}

cMessage *WeightedFairQueue::dequeue()
{
    int selectQueue=-1;
    double endTime =1e20;
    for (int i=0; i<numQueues; i++)
    {
        if (!queueArray[i].empty())
        {
            double time = subqueueData[i].time.front();
            if (time<endTime)
            {
                selectQueue=i;
                endTime=time;
            }
        }
    }
    if (selectQueue!=-1)
    {
        subqueueData[selectQueue].time.pop();
        virt_time=virt_time+(SIMTIME_DBL(simTime())-last_vt_update)/sum;
        last_vt_update=SIMTIME_DBL(simTime());
        //update sum
        if ((--subqueueData[selectQueue].B)==0)
            sum=sum-(double)subqueueData[selectQueue].queueWeight;
        if ( fabs(sum) < safe_limit )
            sum=0;
        if (sum==0)
        {
            GPS_idle=true;
            for (int i=0; i <numQueues; i++) subqueueData[i].finish_t=0;
        }
        lotalLength--;
        emit(queueLengthSignal, lotalLength);
        return (cMessage *)queueArray[selectQueue].pop();
    }
    return NULL;
}

void WeightedFairQueue::sendOut(cMessage *msg)
{
    send(msg, outGate);
}

bool WeightedFairQueue::isEmpty()
{
    for (int i=0; i<numQueues; i++)
    {
    	if (!queueArray[i].empty())
            return false;
    }
    return true;
}

