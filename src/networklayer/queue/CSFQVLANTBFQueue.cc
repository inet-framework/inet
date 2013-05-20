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


#include "CSFQVLANTBFQueue.h"

Define_Module(CSFQVLANTBFQueue);

CSFQVLANTBFQueue::CSFQVLANTBFQueue()
{
//    queues = NULL;
//    numQueues = NULL;
}

CSFQVLANTBFQueue::~CSFQVLANTBFQueue()
{
    for (int i=0; i<numQueues; i++)
    {
        delete queues[i];
        cancelAndDelete(conformityTimer[i]);
    }
}

void CSFQVLANTBFQueue::initialize()
{
    PassiveQueueBase::initialize();

    // configuration
    frameCapacity = par("frameCapacity");
    numQueues = par("numQueues");
    bucketSize = par("bucketSize").longValue()*8LL; // in bit
    meanRate = par("meanRate"); // in bps
    mtu = par("mtu").longValue()*8; // in bit
    peakRate = par("peakRate"); // in bps

    // state
    const char *classifierClass = par("classifierClass");
    classifier = check_and_cast<IQoSClassifier *>(createOne(classifierClass));
    classifier->setMaxNumQueues(numQueues);

    outGate = gate("out");

    // set subqueues
    queues.assign(numQueues, (cQueue *)NULL);
    meanBucketLength.assign(numQueues, bucketSize);
    peakBucketLength.assign(numQueues, mtu);
    lastTime.assign(numQueues, simTime());
    conformityFlag.assign(numQueues, false);
    conformityTimer.assign(numQueues, (cMessage *)NULL);
    for (int i = 0; i < numQueues; i++)
    {
        char buf[32];
        sprintf(buf, "queue-%d", i);
        queues[i] = new cQueue(buf);
        conformityTimer[i] = new cMessage("Conformity Timer", i);   // message kind carries a queue index
    }

    // state: RR scheduler
    currentQueueIndex = 0;

    // statistic
    warmupFinished = false;
    numBitsSent.assign(numQueues, 0.0);
    numQueueReceived.assign(numQueues, 0);
    numQueueDropped.assign(numQueues, 0);
    numQueueUnshaped.assign(numQueues, 0);
    numQueueSent.assign(numQueues, 0);

    // state: flow
    flowState.resize(numQueues);
    for (int i = 0; i < numQueues; i++) {
        flowState[i].weight_    = 1.0;
        flowState[i].k_         = 100000.0;
        flowState[i].numArv_    = flowState[i].numDpt_ = flowState[i].numDroped_ = 0;
        flowState[i].prevTime_  = 0.0;
        flowState[i].estRate_   = 0.0;
        flowState[i].size_      = 0;
#ifdef SRCID
        hashid_[i]              = 0;
#endif
    }

    // CSFQ+TBF
    alpha_ = tmpAlpha_ = 0.;
    kalpha_ = KALPHA;

#ifdef PENALTY_BOX
    for (int i = 0; i < MONITOR_TABLE_SIZE; i++) {
        monitorTable_[i].valid_ = 0;
        monitorTable_[i].prevTime_ = monitorTable_[i].estNormRate_ = 0.;
    }
    for (int i = 0; i < PUNISH_TABLE_SIZE; i++) {
        punishTable_[i].valid_ = 0;
        punishTable_[i].prevTime_ = monitorTable_[i].estNormRate_ = 0.;
    }
    numDroppedPkts_ = 0;
#endif

    lastArv_ = rateAlpha_ = rateTotal_ = 0.;
    lArv_ = 0.;
    pktSize_ = pktSizeE_ = 0;
    congested_ = 1;

#ifdef PENALTY_BOX
    bind("kLink_", &kDropped_);
#endif

//    bind("kLink_", &kLink_);
//    edge_ = 1;
//    bind("edge_", &edge_);
//
//    bind("qsize_", &qsize_);
//    bind("qsizeThresh_", &qsizeThresh_);
//    bind("rate_", &rate_);
//    bind("id_", &id_);
}

void CSFQVLANTBFQueue::handleMessage(cMessage *msg)
{
    if (warmupFinished == false)
    {   // start statistics gathering once the warm-up period has passed.
        if (simTime() >= simulation.getWarmupPeriod()) {
            warmupFinished = true;
            for (int i = 0; i < numQueues; i++)
            {
                numQueueReceived[i] = queues[i]->getLength();   // take into account the frames/packets already in queues
            }
        }
    }

    if (msg->isSelfMessage())
    {   // Conformity Timer expires
        int queueIndex = msg->getKind();    // message kind carries a queue index
        conformityFlag[queueIndex] = true;

        // update TBF status
        int pktLength = (check_and_cast<cPacket *>(queues[queueIndex]->front()))->getBitLength();
        bool conformance = isConformed(queueIndex, pktLength);
// DEBUG
        ASSERT(conformance == true);
// DEBUG
        if (packetRequested > 0)
        {
            cPacket *pkt = check_and_cast<cPacket *>(dequeue());
            if (pkt != NULL)
            {
                packetRequested--;
                if (warmupFinished == true)
                {
                    numBitsSent[currentQueueIndex] += pkt->getBitLength();
                    numQueueSent[currentQueueIndex]++;
                }
                sendOut(pkt);
            }
            else
            {
                error("%s::handleMessage: Error in round-robin scheduling", getFullPath().c_str());
            }
        }
    }
    else
    {   // a frame arrives
        int queueIndex = classifier->classifyPacket(msg);
        cQueue *queue = queues[queueIndex];
        if (warmupFinished == true)
        {
            numQueueReceived[queueIndex]++;
        }
        int pktLength = (check_and_cast<cPacket *>(msg))->getBitLength();
// DEBUG
        ASSERT(pktLength > 0);
// DEBUG
        if (queue->isEmpty())
        {
            if (isConformed(queueIndex, pktLength))
            {
                if (warmupFinished == true)
                {
                    numQueueUnshaped[queueIndex]++;
                }
                if (packetRequested > 0)
                {
                    packetRequested--;
                    if (warmupFinished == true)
                    {
                        numBitsSent[queueIndex] += pktLength;
                        numQueueSent[queueIndex]++;
                    }
                    currentQueueIndex = queueIndex;
                    sendOut(msg);
                }
                else
                {
                    bool dropped = enqueue(msg);
                    if (dropped)
                    {
                        if (warmupFinished == true)
                        {
                            numQueueDropped[queueIndex]++;
                        }
                    }
                    else
                    {
                        conformityFlag[queueIndex] = true;
                    }
                }
            }
            else
            {   // frame is not conformed
                bool dropped = enqueue(msg);
                if (dropped)
                {
                    if (warmupFinished == true)
                    {
                        numQueueDropped[queueIndex]++;
                    }
                }
                else
                {
                    triggerConformityTimer(queueIndex, pktLength);
                    conformityFlag[queueIndex] = false;
                }
            }
        }
        else
        {   // queue is not empty
            bool dropped = enqueue(msg);
            if (dropped)
            {
                if (warmupFinished == true)
                {
                    numQueueDropped[queueIndex]++;
                }
            }
        }

        if (ev.isGUI())
        {
            char buf[40];
            sprintf(buf, "q rcvd: %d\nq dropped: %d", numQueueReceived[queueIndex], numQueueDropped[queueIndex]);
            getDisplayString().setTagArg("t", 0, buf);
        }
    }
}

bool CSFQVLANTBFQueue::enqueue(cMessage *msg)
{
    int queueIndex = classifier->classifyPacket(msg);
    cQueue *queue = queues[queueIndex];

    if (frameCapacity && queue->length() >= frameCapacity)
    {
        EV << "Queue " << queueIndex << " full, dropping packet.\n";
        delete msg;
        return true;
    }
    else
    {
        queue->insert(msg);
        return false;
    }
}

cMessage *CSFQVLANTBFQueue::dequeue()
{
    bool found = false;
    int startQueueIndex = (currentQueueIndex + 1) % numQueues;  // search from the next queue for a frame to transmit
    for (int i = 0; i < numQueues; i++)
    {
       if (conformityFlag[(i+startQueueIndex)%numQueues])
       {
           currentQueueIndex = (i+startQueueIndex)%numQueues;
           found = true;
           break;
       }
    }

    if (found == false)
    {
        // TO DO: further processing?
        return NULL;
    }

    cMessage *msg = (cMessage *)queues[currentQueueIndex]->pop();

    // TO DO: update statistics

    // conformity processing for the HOL frame
    if (queues[currentQueueIndex]->isEmpty() == false)
    {
        int pktLength = (check_and_cast<cPacket *>(queues[currentQueueIndex]->front()))->getBitLength();
        if (isConformed(currentQueueIndex, pktLength))
        {
            conformityFlag[currentQueueIndex] = true;
        }
        else
        {
            conformityFlag[currentQueueIndex] = false;
            triggerConformityTimer(currentQueueIndex, pktLength);
        }
    }
    else
    {
        conformityFlag[currentQueueIndex] = false;
    }

    // return a packet from the scheduled queue for transmission
    return (msg);
}

void CSFQVLANTBFQueue::sendOut(cMessage *msg)
{
    send(msg, outGate);
}

void CSFQVLANTBFQueue::requestPacket()
{
    Enter_Method("requestPacket()");

    cMessage *msg = dequeue();
    if (msg==NULL)
    {
        packetRequested++;
    }
    else
    {
        if (warmupFinished == true)
        {
            numBitsSent[currentQueueIndex] += (check_and_cast<cPacket *>(msg))->getBitLength();
            numQueueSent[currentQueueIndex]++;
        }
        sendOut(msg);
    }
}

bool CSFQVLANTBFQueue::isConformed(int queueIndex, int pktLength)
{
    Enter_Method("isConformed()");

// DEBUG
    EV << "Last Time = " << lastTime[queueIndex] << endl;
    EV << "Current Time = " << simTime() << endl;
    EV << "Packet Length = " << pktLength << endl;
// DEBUG

    // update states
    simtime_t now = simTime();
    //unsigned long long meanTemp = meanBucketLength[queueIndex] + (unsigned long long)(meanRate*(now - lastTime[queueIndex]).dbl() + 0.5);
    unsigned long long meanTemp = meanBucketLength[queueIndex] + (unsigned long long)ceil(meanRate*(now - lastTime[queueIndex]).dbl());
    meanBucketLength[queueIndex] = (long long)((meanTemp > bucketSize) ? bucketSize : meanTemp);
    //unsigned long long peakTemp = peakBucketLength[queueIndex] + (unsigned long long)(peakRate*(now - lastTime[queueIndex]).dbl() + 0.5);
    unsigned long long peakTemp = peakBucketLength[queueIndex] + (unsigned long long)ceil(peakRate*(now - lastTime[queueIndex]).dbl());
    peakBucketLength[queueIndex] = int((peakTemp > mtu) ? mtu : peakTemp);
    lastTime[queueIndex] = now;

    if (pktLength <= meanBucketLength[queueIndex])
    {
        if  (pktLength <= peakBucketLength[queueIndex])
        {
            meanBucketLength[queueIndex] -= pktLength;
            peakBucketLength[queueIndex] -= pktLength;
            return true;
        }
    }
    return false;
}

// trigger TBF conformity timer for the HOL frame in the queue,
// indicating that enough tokens will be available for its transmission
void CSFQVLANTBFQueue::triggerConformityTimer(int queueIndex, int pktLength)
{
    Enter_Method("triggerConformityCounter()");

    double meanDelay = (pktLength - meanBucketLength[queueIndex]) / meanRate;
    double peakDelay = (pktLength - peakBucketLength[queueIndex]) / peakRate;

// DEBUG
    EV << "Packet Length = " << pktLength << endl;
    dumpTbfStatus(queueIndex);
    EV << "Delay for Mean TBF = " << meanDelay << endl;
    EV << "Delay for Peak TBF = " << peakDelay << endl;
    EV << "Current Time = " << simTime() << endl;
    EV << "Counter Expiration Time = " << simTime() + max(meanDelay, peakDelay) << endl;
// DEBUG

    scheduleAt(simTime() + max(meanDelay, peakDelay), conformityTimer[queueIndex]);
}

void CSFQVLANTBFQueue::dumpTbfStatus(int queueIndex)
{
    EV << "Last Time = " << lastTime[queueIndex] << endl;
    EV << "Current Time = " << simTime() << endl;
    EV << "Token bucket for mean rate/burst control " << endl;
    EV << "- Bucket size [bit]: " << bucketSize << endl;
    EV << "- Mean rate [bps]: " << meanRate << endl;
    EV << "- Bucket length [bit]: " << meanBucketLength[queueIndex] << endl;
    EV << "Token bucket for peak rate/MTU control " << endl;
    EV << "- MTU [bit]: " << mtu << endl;
    EV << "- Peak rate [bps]: " << peakRate << endl;
    EV << "- Bucket length [bit]: " << peakBucketLength[queueIndex] << endl;
}

// compute estimated flow rate by using exponentially averaging
double CSFQVLANTBFQueue::estRate(int flowId, int pktSize, double arrvTime)
{
    double d = (arvTime - fs_[flowid].prevTime_) * 1000000;
    double k = fs_[flowid].k_;

    if (d == 0.0)
    {
        fs_[flowid].size_ += pktSize;
        if (fs_[flowid].estRate_)
        {
            return fs_[flowid].estRate_;
        }
        else
        {   // this is the first packet; just initialize the rate
            return (fs_[flowid].estRate_ = rateAlpha_ / 2);
        }
    }
    else
    {
        pktSize += fs_[flowid].size_;
        fs_[flowid].size_ = 0;
    }

    fs_[flowid].prevTime_ = arvTime;
    fs_[flowid].estRate_ = (1. - exp(-d / k)) * (double) pktSize / d + exp(-d / k) * fs_[flowid].estRate_;
    return fs_[flowid].estRate_;
}

// estimate the link's alpha parameter
void CSFQVLANTBFQueue::estAlpha(int pktSize, double pktLabel, double arvTime, int enqueue)
{
    float d = (arvTime - lastArv_) * 1000000., w, rate = rate_ / 1000000.;
    double k = kLink_ / 1000000.;

    // set lastArv_ to the arrival time of the first packet
    if (lastArv_ == 0.)
    {
        lastArv_ = arvTime;
    }

    // account for packets received simultaneously
    pktSize_ += pktSize;
    if (enqueue)
    {
        pktSizeE_ += pktSize;
    }
    if (arvTime == lastArv_)
    {
        return;
    }

    // estimate the aggreagte arrival rate (rateTotal_) and the
    // aggregate forwarded (rateAlpha_) rates
    w = exp(-d / kLink_);
    rateAlpha_ = (1 - w) * (double) pktSizeE_ / d + w * rateAlpha_;
    rateTotal_ = (1 - w) * (double) pktSize_ / d + w * rateTotal_;
    lastArv_ = arvTime;
    pktSize_ = pktSizeE_ = 0;

    // compute the initial value of alpha_
    if (alpha_ == 0.)
    {
        if (qsizeCrt_ < qsizeThresh_)
        {
            if (tmpAlpha_ < pktLabel)
            {
                tmpAlpha_ = pktLabel;
            }
            return;
        }
        if (alpha_ < tmpAlpha_)
        {
            alpha_ = tmpAlpha_;
        }
        if (alpha_ == 0.)
        {
            alpha_ = rate / 2.;  // arbitrary initialization
        }
        tmpAlpha_ = 0.;
    }
    // update alpha_
    if (rate <= rateTotal_)
    { // link congested
        if (!congested_)
        {
            congested_ = 1;
            lArv_ = arvTime;
            kalpha_ = KALPHA;
        }
        else
        {
            if (arvTime < lArv_ + k)
            {
                return;
            }
            lArv_ = arvTime;
            alpha_ *= rate / rateAlpha_;
            if (rate < alpha_)
            {
                alpha_ = rate;
            }
        }
    }
    else
    {   // (rate < rateTotal_) => link uncongested
        if (congested_)
        {
            congested_ = 0;
            lArv_ = arvTime;
            tmpAlpha_ = 0;
        }
        else
        {
            if (arvTime < lArv_ + k)
            {
                if (tmpAlpha_ < pktLabel)
                {
                    tmpAlpha_ = pktLabel;
                }
            }
            else
            {
                alpha_ = tmpAlpha_;
                lArv_ = arvTime;
                if (qsizeCrt_ < qsizeThresh_)
                {
                    alpha_ = 0.;
                }
                else
                {
                    tmpAlpha_ = 0.;
                }
            }
        }
    }

#ifdef CSFQ_LOG
    printf("|%d %f %f %f\n", id_, arvTime, rateAlpha_, rateTotal_);
#endif
}

void CSFQVLANTBFQueue::finish()
{
    unsigned long sumQueueReceived = 0;
    unsigned long sumQueueDropped = 0;
    unsigned long sumQueueShaped = 0;
    unsigned long sumQueueUnshaped = 0;

    for (int i=0; i < numQueues; i++)
    {
        std::stringstream ss_received, ss_dropped, ss_shaped, ss_sent, ss_throughput;
        ss_received << "packets received by per-VLAN queue[" << i << "]";
        ss_dropped << "packets dropped by per-VLAN queue[" << i << "]";
        ss_shaped << "packets shaped by per-VLAN queue[" << i << "]";
        ss_sent << "packets sent by per-VLAN queue[" << i << "]";
        ss_throughput << "bits/sec sent per-VLAN queue[" << i << "]";
        recordScalar((ss_received.str()).c_str(), numQueueReceived[i]);
        recordScalar((ss_dropped.str()).c_str(), numQueueDropped[i]);
        recordScalar((ss_shaped.str()).c_str(), numQueueReceived[i]-numQueueUnshaped[i]);
        recordScalar((ss_sent.str()).c_str(), numQueueSent[i]);
        recordScalar((ss_throughput.str()).c_str(), numBitsSent[i]/(simTime()-simulation.getWarmupPeriod()).dbl());
        sumQueueReceived += numQueueReceived[i];
        sumQueueDropped += numQueueDropped[i];
        sumQueueShaped += numQueueReceived[i] - numQueueUnshaped[i];
    }
    recordScalar("overall packet loss rate of per-VLAN queues", sumQueueDropped/double(sumQueueReceived));
    recordScalar("overall packet shaped rate of per-VLAN queues", sumQueueShaped/double(sumQueueReceived));
}
