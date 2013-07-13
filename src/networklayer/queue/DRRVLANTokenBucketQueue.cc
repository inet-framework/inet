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


#include "DRRVLANTokenBucketQueue.h"

Define_Module(DRRVLANTokenBucketQueue);

DRRVLANTokenBucketQueue::DRRVLANTokenBucketQueue()
{
//    queues = NULL;
//    numFlows = NULL;
}

DRRVLANTokenBucketQueue::~DRRVLANTokenBucketQueue()
{
    for (int i=0; i<numFlows; i++)
    {
        delete queues[i];
    }
}

void DRRVLANTokenBucketQueue::initialize()
{
    PassiveQueueBase::initialize();

    outGate = gate("out");

    // general
    numFlows = par("numFlows");
    queueSize = par("queueSize");

    // VLAN classifier
    const char *classifierClass = par("classifierClass");
    classifier = check_and_cast<IQoSClassifier *>(createOne(classifierClass));
    classifier->setMaxNumQueues(numFlows);
    const char *vids = par("vids");
    classifier->initialize(vids);

    // Token bucket meters
    tbm.assign(numFlows, (BasicTokenBucketMeter *)NULL);
    for (int i=0; i<numFlows; i++)
    {
        tbm[i] = check_and_cast<BasicTokenBucketMeter *>(this->getSubmodule("tbm", i));
    }

    // FIFO queue for conformant packets
    fifo.setName("FIFO queue");

    // Per-subscriber queues with DRR scheduler for non-conformant packets
    queues.assign(numFlows, (cQueue *)NULL);
    for (int i=0; i<numFlows; i++)
    {
        char buf[32];
        sprintf(buf, "queue-%d", i);
        queues[i] = new cQueue(buf);
    }

    // DRR scheduler
    currentQueueIndex = 0;

    // statistics
    warmupFinished = false;
    numBitsSent.assign(numFlows, 0.0);
    numPktsReceived.assign(numFlows, 0);
    numPktsDropped.assign(numFlows, 0);
    numPktsConformed.assign(numFlows, 0);
    numPktsSent.assign(numFlows, 0);
}

void DRRVLANTokenBucketQueue::handleMessage(cMessage *msg)
{
    if (warmupFinished == false)
    {   // start statistics gathering once the warm-up period has passed.
        if (simTime() >= simulation.getWarmupPeriod()) {
            warmupFinished = true;
            for (int i = 0; i < numFlows; i++)
            {
                numPktsReceived[i] = queues[i]->getLength();   // take into account the frames/packets already in queues
            }
        }
    }

    if (!msg->isSelfMessage())
    {   // a frame arrives
        int flowIndex = classifier->classifyPacket(msg);
        cQueue *queue = queues[flowIndex];
        if (warmupFinished == true)
        {
            numPktsReceived[flowIndex]++;
        }
        int pktLength = (check_and_cast<cPacket *>(msg))->getBitLength();
// DEBUG
        ASSERT(pktLength > 0);
// DEBUG

        if (tbm[flowIndex]->meterPacket(msg) == 0)
        {   // conformant packet
            if (warmupFinished == true)
            {
                numPktsConformed[flowIndex]++;
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
                if (queueSize && fifo.length() >= queueSize)
                {
                    EV << "FIFO queue full, dropping packet.\n";
                    if (warmupFinished == true)
                    {
                        numPktsDropped[flowIndex]++;
                    }
                    delete msg;
                }
                else
                {
                    fifo.insert(msg);
                }
            }
        }
        else
        {   // non-conformant packet
            if (queue->isEmpty())
            {
                bool dropped = enqueue(msg);
                if (dropped)
                {
                    if (warmupFinished == true)
                    {
                        numPktsDropped[flowIndex]++;
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
    else
    {   // self message is not expected; something wrong.
        error("%s::handleMessage: Unexpected self message", getFullPath().c_str());
    }
}

bool DRRVLANTokenBucketQueue::enqueue(cMessage *msg)
{
    int queueIndex = classifier->classifyPacket(msg);
    cQueue *queue = queues[queueIndex];

    if (queueSize && queue->length() >= queueSize)
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

cMessage *DRRVLANTokenBucketQueue::dequeue()
{
    if (!fifo.isEmpty())
    {
        return ((cMessage *)fifo.pop());
    }
    else
    {   // TODO: implement DRR scheduling here
        bool found = false;
        int startQueueIndex = (currentQueueIndex + 1) % numFlows;  // search from the next queue for a frame to transmit
        for (int i = 0; i < numFlows; i++)
        {
            currentQueueIndex = (i + startQueueIndex) % numFlows;
            found = true;
            break;
        }

        if (found)
        {
            // TODO: update statistics
            return ((cMessage *)queues[currentQueueIndex]->pop());
        }
        else
        {
            // TODO: further processing?
            return NULL;
        }
    }
}

void DRRVLANTokenBucketQueue::sendOut(cMessage *msg)
{
    send(msg, outGate);
}

void DRRVLANTokenBucketQueue::requestPacket()
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
            int flowIndex = classifier->classifyPacket(msg);    // TODO: any more efficient way?
            numBitsSent[flowIndex] += (check_and_cast<cPacket *>(msg))->getBitLength();
            numPktsSent[flowIndex]++;
        }
        sendOut(msg);
    }
}

void DRRVLANTokenBucketQueue::finish()
{
    unsigned long sumQueueReceived = 0;
    unsigned long sumQueueDropped = 0;
    unsigned long sumQueueShaped = 0;
    unsigned long sumQueueUnshaped = 0;

    for (int i=0; i < numFlows; i++)
    {
        std::stringstream ss_received, ss_dropped, ss_shaped, ss_sent, ss_throughput;
        ss_received << "packets received by per-VLAN queue[" << i << "]";
        ss_dropped << "packets dropped by per-VLAN queue[" << i << "]";
        ss_shaped << "packets shaped by per-VLAN queue[" << i << "]";
        ss_sent << "packets sent by per-VLAN queue[" << i << "]";
        ss_throughput << "bits/sec sent per-VLAN queue[" << i << "]";
        recordScalar((ss_received.str()).c_str(), numPktsReceived[i]);
        recordScalar((ss_dropped.str()).c_str(), numPktsDropped[i]);
        recordScalar((ss_shaped.str()).c_str(), numPktsReceived[i]-numPktsConformed[i]);
        recordScalar((ss_sent.str()).c_str(), numPktsSent[i]);
        recordScalar((ss_throughput.str()).c_str(), numBitsSent[i]/(simTime()-simulation.getWarmupPeriod()).dbl());
        sumQueueReceived += numPktsReceived[i];
        sumQueueDropped += numPktsDropped[i];
        sumQueueShaped += numPktsReceived[i] - numPktsConformed[i];
    }
    recordScalar("overall packet loss rate of per-VLAN queues", sumQueueDropped/double(sumQueueReceived));
    recordScalar("overall packet shaped rate of per-VLAN queues", sumQueueShaped/double(sumQueueReceived));
}
