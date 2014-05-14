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


#include "DropTailVLANTBFQueue.h"

Define_Module(DropTailVLANTBFQueue);

DropTailVLANTBFQueue::DropTailVLANTBFQueue()
{
//    queues = NULL;
//    numPkts = NULL;
}

DropTailVLANTBFQueue::~DropTailVLANTBFQueue()
{
    for (int i=0; i<numQueues; i++)
    {
        delete queues[i];
        cancelAndDelete(conformityTimer[i]);
    }
}

void DropTailVLANTBFQueue::initialize()
{
    PassiveQueueBase::initialize();

    // configuration
    fifoCapacity = par("fifoCapacity");
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

    // set common FIFO queue
    fifo.setName("FIFO queue");

    // set subqueues
    queues.assign(numQueues, (cQueue *)NULL);
    meanBucketLength.assign(numQueues, bucketSize);
    peakBucketLength.assign(numQueues, mtu);
    lastTime.assign(numQueues, simTime());
    conformityFlag.assign(numQueues, false);
    conformityTimer.assign(numQueues, (cMessage *)NULL);
    for (int i=0; i<numQueues; i++)
    {
        char buf[32];
        sprintf(buf, "queue-%d", i);
        queues[i] = new cQueue(buf);
        conformityTimer[i] = new cMessage("Conformity Timer", i);   // message kind carries a flow index
    }

    // // state: RR scheduler
    // currentQueueIndex = 0;

    // statistic
    warmupFinished = false;
    numBitsSent.assign(numQueues, 0.0);
    numPktsReceived.assign(numQueues, 0);
    numPktsDropped.assign(numQueues, 0);
    numPktsUnshaped.assign(numQueues, 0);
    numPktsSent.assign(numQueues, 0);
}

void DropTailVLANTBFQueue::handleMessage(cMessage *msg)
{
    if (warmupFinished == false)
    {   // start statistics gathering once the warm-up period has passed.
        if (simTime() >= simulation.getWarmupPeriod()) {
            warmupFinished = true;
            for (int i = 0; i < numQueues; i++)
            {
                numPktsReceived[i] = queues[i]->getLength();   // take into account the frames/packets already in queues
            }
        }
    }

    if (msg->isSelfMessage())
    {   // Conformity Timer expires
        int flowIndex = msg->getKind();    // message kind carries a flow index
        conformityFlag[flowIndex] = true;

        cPacket *pkt = check_and_cast<cPacket *>(dequeue());
        int pktLength = pkt->getBitLength();
        bool conformance = isConformed(flowIndex, pktLength);  // update TBF status
// DEBUG
        ASSERT(conformance == true);
// DEBUG
        if (pkt != NULL)
        {
            if (packetRequested > 0)
            {
                packetRequested--;
                if (warmupFinished == true)
                {
                    numBitsSent[flowIndex] += pkt->getBitLength();
                    numPktsSent[flowIndex]++;
                }
                sendOut(pkt);
            }
            else
            {
                bool dropped = enqueue(pkt);
                if (dropped)
                {
                    if (warmupFinished == true)
                    {
                        numPktsDropped[flowIndex]++;
                    }
                }
            }
        }
        else
        {
            error("%s::handleMessage: Error in FIFO scheduling", getFullPath().c_str());
        }
    }
    else
    {   // a frame arrives
        int flowIndex = classifier->classifyPacket(msg);
        cQueue *queue = queues[flowIndex];
        if (warmupFinished == true)
        {
            numPktsReceived[flowIndex]++;
        }
        int pktLength = PK(msg)->getBitLength();
// DEBUG
        ASSERT(pktLength > 0);
// DEBUG
        if (queue->isEmpty())
        {
            if (isConformed(flowIndex, pktLength))
            {
                if (warmupFinished == true)
                {
                    numPktsUnshaped[flowIndex]++;
                }
                if (packetRequested > 0)
                {
                    packetRequested--;
                    if (warmupFinished == true)
                    {
                        numBitsSent[flowIndex] += pktLength;
                        numPktsSent[flowIndex]++;
                    }
                    sendOut(msg);
                }
                else
                {
                    bool dropped = enqueue(msg);
                    if (dropped)
                    {
                        if (warmupFinished == true)
                        {
                            numPktsDropped[flowIndex]++;
                        }
                    }
                    else
                    {
                        conformityFlag[flowIndex] = true;
                    }
                }
            }
            else
            {   // frame is not conformed
                bool dropped = voqEnqueue(msg);
                if (dropped)
                {
                    if (warmupFinished == true)
                    {
                        numPktsDropped[flowIndex]++;
                    }
                }
                else
                {
                    triggerConformityTimer(flowIndex, pktLength);
                    conformityFlag[flowIndex] = false;
                }
            }
        }
        else
        {   // queue is not empty
            bool dropped = voqEnqueue(msg);
            if (dropped)
            {
                if (warmupFinished == true)
                {
                    numPktsDropped[flowIndex]++;
                }
            }
        }

        if (ev.isGUI())
        {
            char buf[40];
            sprintf(buf, "q rcvd: %d\nq dropped: %d", numPktsReceived[flowIndex], numPktsDropped[flowIndex]);
            getDisplayString().setTagArg("t", 0, buf);
        }
    }
}

bool DropTailVLANTBFQueue::enqueue(cMessage *msg)
{
    if (frameCapacity && fifo.length() >= frameCapacity)
    {
        EV << "FIFO full, dropping packet.\n";
        delete msg;
        return true;
    }
    else
    {
        fifo.insert(msg);
        return false;
    }
}

bool DropTailVLANTBFQueue::voqEnqueue(cMessage *msg)
{
    int flowIndex = classifier->classifyPacket(msg);
    cQueue *queue = queues[flowIndex];

    if (frameCapacity && queue->length() >= frameCapacity)
    {
        EV << "Queue " << flowIndex << " full, dropping packet.\n";
        delete msg;
        return true;
    }
    else
    {
        queue->insert(msg);
        return false;
    }
}

cMessage *DropTailVLANTBFQueue::dequeue()
{
    if (fifo.empty())
    {
        return NULL;
    }

    cMessage *msg = (cMessage *)fifo.pop();
    
    return msg;
    // bool found = false;
    // int startQueueIndex = (currentQueueIndex + 1) % numQueues;  // search from the next queue for a frame to transmit
    // for (int i = 0; i < numQueues; i++)
    // {
    //    if (conformityFlag[(i+startQueueIndex)%numQueues])
    //    {
    //        currentQueueIndex = (i+startQueueIndex)%numQueues;
    //        found = true;
    //        break;
    //    }
    // }

    // if (found == false)
    // {
    //     // TO DO: further processing?
    //     return NULL;
    // }

    // cMessage *msg = (cMessage *)queues[currentQueueIndex]->pop();

    // // TO DO: update statistics

    // // conformity processing for the HOL frame
    // if (queues[currentQueueIndex]->isEmpty() == false)
    // {
    //     int pktLength = (check_and_cast<cPacket *>(queues[currentQueueIndex]->front()))->getBitLength();
    //     if (isConformed(currentQueueIndex, pktLength))
    //     {
    //         conformityFlag[currentQueueIndex] = true;
    //     }
    //     else
    //     {
    //         conformityFlag[currentQueueIndex] = false;
    //         triggerConformityTimer(currentQueueIndex, pktLength);
    //     }
    // }
    // else
    // {
    //     conformityFlag[currentQueueIndex] = false;
    // }

    // // return a packet from the scheduled queue for transmission
    // return (msg);
}

void DropTailVLANTBFQueue::sendOut(cMessage *msg)
{
    send(msg, outGate);
}

void DropTailVLANTBFQueue::requestPacket()
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
            int flowIndex = classifier->classifyPacket(msg);
            numBitsSent[flowIndex] += PK(msg)->getBitLength();
            numPktsSent[flowIndex]++;
        }
        sendOut(msg);
    }
}

bool DropTailVLANTBFQueue::isConformed(int flowIndex, int pktLength)
{
    Enter_Method("isConformed()");

// DEBUG
    EV << "Last Time = " << lastTime[flowIndex] << endl;
    EV << "Current Time = " << simTime() << endl;
    EV << "Packet Length = " << pktLength << endl;
// DEBUG

    // update states
    simtime_t now = simTime();
    //unsigned long long meanTemp = meanBucketLength[flowIndex] + (unsigned long long)(meanRate*(now - lastTime[flowIndex]).dbl() + 0.5);
    unsigned long long meanTemp = meanBucketLength[flowIndex] + (unsigned long long)ceil(meanRate*(now - lastTime[flowIndex]).dbl());
    meanBucketLength[flowIndex] = (unsigned long long)((meanTemp > bucketSize) ? bucketSize : meanTemp);
    //unsigned long long peakTemp = peakBucketLength[flowIndex] + (unsigned long long)(peakRate*(now - lastTime[flowIndex]).dbl() + 0.5);
    unsigned long long peakTemp = peakBucketLength[flowIndex] + (unsigned long long)ceil(peakRate*(now - lastTime[flowIndex]).dbl());
    peakBucketLength[flowIndex] = int((peakTemp > (unsigned long long)mtu) ? mtu : peakTemp);
    lastTime[flowIndex] = now;

    if ((unsigned long long) pktLength <= meanBucketLength[flowIndex])
    {
        if  (pktLength <= peakBucketLength[flowIndex])
        {
            meanBucketLength[flowIndex] -= pktLength;
            peakBucketLength[flowIndex] -= pktLength;
            return true;
        }
    }
    return false;
}

// trigger TBF conformity timer for the HOL frame in the queue,
// indicating that enough tokens will be available for its transmission
void DropTailVLANTBFQueue::triggerConformityTimer(int flowIndex, int pktLength)
{
    Enter_Method("triggerConformityCounter()");

    double meanDelay = 0.0;
    if ((unsigned long long)pktLength > meanBucketLength[flowIndex]) {
        meanDelay = (pktLength - meanBucketLength[flowIndex]) / meanRate;
    }
    double peakDelay = 0.0;
    if (pktLength > peakBucketLength[flowIndex]) {
        peakDelay = (pktLength - peakBucketLength[flowIndex]) / peakRate;
    }

// DEBUG
    EV << "** For VOQ[" << flowIndex << "]:" << endl;
    EV << "Packet Length = " << pktLength << endl;
    EV << "Delay for Mean TBF = " << meanDelay << endl;
    EV << "Delay for Peak TBF = " << peakDelay << endl;
    EV << "Current Time = " << simTime() << endl;
    EV << "Counter Expiration Time = " << simTime() + std::max(meanDelay, peakDelay) << endl;
    dumpTbfStatus(flowIndex);
// DEBUG

    scheduleAt(simTime() + std::max(meanDelay, peakDelay), conformityTimer[flowIndex]);
}

void DropTailVLANTBFQueue::dumpTbfStatus(int flowIndex)
{
    EV << "Last Time = " << lastTime[flowIndex] << endl;
    EV << "Current Time = " << simTime() << endl;
    EV << "Token bucket for mean rate/burst control " << endl;
    EV << "- Bucket size [bit]: " << bucketSize << endl;
    EV << "- Mean rate [bps]: " << meanRate << endl;
    EV << "- Bucket length [bit]: " << meanBucketLength[flowIndex] << endl;
    EV << "Token bucket for peak rate/MTU control " << endl;
    EV << "- MTU [bit]: " << mtu << endl;
    EV << "- Peak rate [bps]: " << peakRate << endl;
    EV << "- Bucket length [bit]: " << peakBucketLength[flowIndex] << endl;
}

void DropTailVLANTBFQueue::finish()
{
    unsigned long sumQueueReceived = 0;
    unsigned long sumQueueDropped = 0;
    unsigned long sumQueueShaped = 0;

    for (int i=0; i < numQueues; i++)
    {
        std::stringstream ss_received, ss_dropped, ss_shaped, ss_sent, ss_throughput;
        ss_received << "packets received by per-VLAN queue[" << i << "]";
        ss_dropped << "packets dropped by per-VLAN queue[" << i << "]";
        ss_shaped << "packets shaped by per-VLAN queue[" << i << "]";
        ss_sent << "packets sent by per-VLAN queue[" << i << "]";
        ss_throughput << "bits/sec sent per-VLAN queue[" << i << "]";
        recordScalar((ss_received.str()).c_str(), numPktsReceived[i]);
        recordScalar((ss_dropped.str()).c_str(), numPktsDropped[i]);
        recordScalar((ss_shaped.str()).c_str(), numPktsReceived[i]-numPktsUnshaped[i]);
        recordScalar((ss_sent.str()).c_str(), numPktsSent[i]);
        recordScalar((ss_throughput.str()).c_str(), numBitsSent[i]/(simTime()-simulation.getWarmupPeriod()).dbl());
        sumQueueReceived += numPktsReceived[i];
        sumQueueDropped += numPktsDropped[i];
        sumQueueShaped += numPktsReceived[i] - numPktsUnshaped[i];
    }
    recordScalar("overall packet loss rate of per-VLAN queues", sumQueueDropped/double(sumQueueReceived));
    recordScalar("overall packet shaped rate of per-VLAN queues", sumQueueShaped/double(sumQueueReceived));
}
