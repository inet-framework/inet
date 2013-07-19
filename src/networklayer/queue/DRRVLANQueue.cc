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


#include "DRRVLANQueue.h"

Define_Module(DRRVLANQueue);

DRRVLANQueue::DRRVLANQueue()
{
}

DRRVLANQueue::~DRRVLANQueue()
{
    for (int i=0; i<numFlows; i++)
    {
        delete queues[i];
    }
}

void DRRVLANQueue::initialize()
{
    PassiveQueueBase::initialize();

    outGate = gate("out");

    // general
    numFlows = par("numFlows").longValue();
    frameCapacity = par("frameCapacity").longValue();
//    queueSize = par("queueSize");

    // VLAN classifier
    const char *classifierClass = par("classifierClass").stringValue();
    classifier = check_and_cast<IQoSClassifier *>(createOne(classifierClass));
    classifier->setMaxNumQueues(numFlows);
    const char *vids = par("vids").stringValue();
    classifier->initialize(vids);

    // Token bucket meters
    tbm.assign(numFlows, (BasicTokenBucketMeter *)NULL);
    cModule *mac = getParentModule();
    for (int i=0; i<numFlows; i++)
    {
        cModule *meter = mac->getSubmodule("meter", i);
        tbm[i] = check_and_cast<BasicTokenBucketMeter *>(meter);
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
    deficitCounters.assign(numFlows, 0);

    quanta.assign(numFlows, 0);
    const char *str = par("quanta");
    cStringTokenizer tokenizer(str);
    int idx = 0;
    while (tokenizer.hasMoreTokens())
    {
        if (idx >= numFlows)
        {
            throw cRuntimeError("%s::initialize DRR quanta: Exceeds the maximum number of flows", getFullPath().c_str());
        }
        else
        {
            const char *token = tokenizer.nextToken();
            quanta[idx] = atoi(token);
            idx++;
        }
    }

    // statistics
    warmupFinished = false;
    numBitsSent.assign(numFlows, 0.0);
    numPktsReceived.assign(numFlows, 0);
    numPktsDropped.assign(numFlows, 0);
    numPktsConformed.assign(numFlows, 0);
    numPktsSent.assign(numFlows, 0);
}

void DRRVLANQueue::handleMessage(cMessage *msg)
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
        int pktLength = PK(msg)->getBitLength();
// DEBUG
        ASSERT(pktLength > 0);
// DEBUG
        cQueue *queue = queues[flowIndex];
        if (warmupFinished == true)
        {
            numPktsReceived[flowIndex]++;
        }
        if (tbm[flowIndex]->meterPacket(msg) == 0)
        {   // frame is conformed
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
                if (frameCapacity && fifo.length() >= frameCapacity)
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
        {   // frame is not conformed
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

bool DRRVLANQueue::enqueue(cMessage *msg)
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

cMessage *DRRVLANQueue::dequeue()
{
    if (!fifo.isEmpty())
    {   // FIFO queue has a priority over per-flow queues
        return ((cMessage *)fifo.pop());
    }
    else
    {   // DRR scheduling over per-flow queues
        bool found = false;
        int startQueueIndex = (currentQueueIndex + 1) % numFlows;  // search from the next queue for a frame to transmit
        for (int i = 0; i < numFlows; i++)
        {
            int idx = (i + startQueueIndex) % numFlows;
            if (!queues[idx]->isEmpty())
            {
                deficitCounters[idx] += quanta[idx];
                int pktLength = PK(queues[idx]->front())->getByteLength();
                if (deficitCounters[idx] >= pktLength)
                {
                    currentQueueIndex = idx;
                    deficitCounters[idx] -= pktLength;
                    found = true;
                    break;
                }
            }
        } // end of for()

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

void DRRVLANQueue::sendOut(cMessage *msg)
{
    send(msg, outGate);
}

void DRRVLANQueue::requestPacket()
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
            numBitsSent[flowIndex] += PK(msg)->getBitLength();
            numPktsSent[flowIndex]++;
        }
        sendOut(msg);
    }
}

void DRRVLANQueue::finish()
{
    unsigned long sumPktsReceived = 0;
    unsigned long sumPktsDropped = 0;
    unsigned long sumPktsNonconformed = 0;

    for (int i=0; i < numFlows; i++)
    {
        std::stringstream ss_received, ss_dropped, ss_nonconformed, ss_sent, ss_throughput;
        ss_received << "packets received from flow[" << i << "]";
        ss_dropped << "packets dropped from flow[" << i << "]";
        ss_nonconformed << "packets non-conformed from flow[" << i << "]";
        ss_sent << "packets sent from flow[" << i << "]";
        ss_throughput << "bits/sec sent from flow[" << i << "]";
        recordScalar((ss_received.str()).c_str(), numPktsReceived[i]);
        recordScalar((ss_dropped.str()).c_str(), numPktsDropped[i]);
        recordScalar((ss_nonconformed.str()).c_str(), numPktsReceived[i]-numPktsConformed[i]);
        recordScalar((ss_sent.str()).c_str(), numPktsSent[i]);
        recordScalar((ss_throughput.str()).c_str(), numBitsSent[i]/(simTime()-simulation.getWarmupPeriod()).dbl());
        sumPktsReceived += numPktsReceived[i];
        sumPktsDropped += numPktsDropped[i];
        sumPktsNonconformed += numPktsReceived[i] - numPktsConformed[i];
    }
    recordScalar("overall packet loss rate of per-VLAN queues", sumPktsDropped/double(sumPktsReceived));
    recordScalar("overall packet shaped rate of per-VLAN queues", sumPktsNonconformed/double(sumPktsReceived));
}
