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
        delete voq[i];
    }
}

void DRRVLANQueue::initialize()
{
    PassiveQueueBase::initialize();

    outGate = gate("out");

    // general
    numFlows = par("numFlows").longValue();

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
    fifoSize = par("fifoSize").longValue();
    fifo.setName("FIFO queue");
    fifoCurrentSize = 0;

    // Per-subscriber VOQs with DRR scheduler for non-conformant packets
    voqSize = par("voqSize").longValue();
    voq.assign(numFlows, (cQueue *)NULL);
    for (int i=0; i<numFlows; i++)
    {
        char buf[32];
        sprintf(buf, "queue-%d", i);
        voq[i] = new cQueue(buf);
    }
    voqCurrentSize.assign(numFlows, 0);

    // DRR scheduler
    currentVoqIndex = 0;
    deficitCounters.assign(numFlows, 0);
    continuation = false;

    // DRR scheduler: Quanta
    const char *str = par("quanta");
    cStringTokenizer tokenizer(str);
    quanta = tokenizer.asIntVector();
    if (quanta.size() < numFlows)
    {
        int quantum = *(quanta.end() - 1);  // fill new elements with the value of the last element
        quanta.resize(numFlows, quantum);
    }
    else if (quanta.size() > numFlows)
    {
        error("%s::initialize DRR quanta: Exceeds the maximum number of flows", getFullPath().c_str());
    }

    // statistics
    warmupFinished = false;
    numBitsSent.assign(numFlows, 0.0);
    numPktsReceived.assign(numFlows, 0);
    numPktsDropped.assign(numFlows, 0);
    numPktsConformed.assign(numFlows, 0);
    numPktsSent.assign(numFlows, 0);

    // debugging
#ifndef NDEBUG
    pktReqVector.setName("packets requested");
#endif
}

void DRRVLANQueue::handleMessage(cMessage *msg)
{
    if (warmupFinished == false)
    {   // start statistics gathering once the warm-up period has passed.
        if (simTime() >= simulation.getWarmupPeriod()) {
            warmupFinished = true;
            for (int i = 0; i < numFlows; i++)
            {
                numPktsReceived[i] = voq[i]->getLength();   // take into account the frames/packets already in VOQs
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
        int pktByteLength = PK(msg)->getByteLength();
        int color = tbm[flowIndex]->meterPacket(msg);   // result of metering; 0 for conformed and 1 for non-conformed packet
        cQueue *queue = voq[flowIndex];
        if (warmupFinished == true)
        {
            numPktsReceived[flowIndex]++;
            if (color == 0)
            {   // frame is conformed
                numPktsConformed[flowIndex]++;
            }
        }

        if (packetRequested > 0)
        {
            packetRequested--;
#ifndef NDEBUG
            pktReqVector.record(packetRequested);
#endif
            if (warmupFinished == true)
            {
                numBitsSent[flowIndex] += pktLength;
                numPktsSent[flowIndex]++;
            }
            sendOut(msg);
        }
        else
        {
            if (color == 0)
            {   // frame is conformed
                if (fifoCurrentSize + pktByteLength > fifoSize)
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
                    fifoCurrentSize += pktByteLength;
                }
            }
//        if (tbm[flowIndex]->meterPacket(msg) == 0)
//        {   // frame is conformed
//            if (warmupFinished == true)
//            {
//                numPktsConformed[flowIndex]++;
//            }
//            if (packetRequested > 0)
//            {
//                packetRequested--;
//#ifndef NDEBUG
//                pktReqVector.record(packetRequested);
//#endif
//                if (warmupFinished == true)
//                {
//                    numBitsSent[flowIndex] += pktLength;
//                    numPktsSent[flowIndex]++;
//                }
//                sendOut(msg);
//            }
//            else
//            {
//                int pktByteLength = PK(msg)->getByteLength();
//                if (fifoCurrentSize + pktByteLength > fifoSize)
//                {
//                    EV << "FIFO queue full, dropping packet.\n";
//                    if (warmupFinished == true)
//                    {
//                        numPktsDropped[flowIndex]++;
//                    }
//                    delete msg;
//                }
//                else
//                {
//                    fifo.insert(msg);
//                    fifoCurrentSize += pktByteLength;
//                }
//            }
//        }
            else
            {   // frame is not conformed
                bool dropped = enqueue(msg);
                if (dropped)
                {
                    if (warmupFinished == true)
                    {
                        numPktsDropped[flowIndex]++;
                    }
                }
            }
        }

        if (ev.isGUI())
        {
            char buf[40];
            sprintf(buf, "q rcvd: %d\nq dropped: %d", numPktsReceived[flowIndex], numPktsDropped[flowIndex]);
            getDisplayString().setTagArg("t", 0, buf);
        }
    }   // end of if () for self message
    else
    {   // self message is not expected; something wrong.
        error("%s::handleMessage: Unexpected self message", getFullPath().c_str());
    }
}

bool DRRVLANQueue::enqueue(cMessage *msg)
{
    int queueIndex = classifier->classifyPacket(msg);
    int pktByteLength = PK(msg)->getByteLength();

    if (voqCurrentSize[queueIndex] + pktByteLength > voqSize)
    {
        EV << "VOQ[" << queueIndex << "] full, dropping packet.\n";
        delete msg;
        return true;
    }
    else
    {
        voq[queueIndex]->insert(msg);
        voqCurrentSize[queueIndex] += pktByteLength;
        return false;
    }
}

cMessage *DRRVLANQueue::dequeue()
{
    cMessage *msg = (cMessage *)NULL;

    if (!fifo.isEmpty())
    {   // FIFO queue has a priority over per-flow VOQs
        msg = (cMessage *)fifo.pop();
        fifoCurrentSize -= PK(msg)->getByteLength();
        return (msg);
    }
    else
    {   // DRR scheduling over per-flow VOQs

        // check whether there is any non-empty VOQ
        bool isVoqEmpty = true;
        for (int i = 0; i < numFlows; i++)
        {
            if (!voq[i]->isEmpty())
            {
                isVoqEmpty = false;
                break;
            }
        }

        if (isVoqEmpty)
        {
            continuation = false;
            return (msg);
        }
        else
        {
            int i = 0;
            while (true)
            {
                int idx = (currentVoqIndex + i) % numFlows;
                if (!voq[idx]->isEmpty())
                {
                    deficitCounters[idx] += continuation ? 0 : quanta[idx];
                    continuation = false;   // reset the flag

                    int pktByteLength = PK(voq[idx]->front())->getByteLength();
                    if (deficitCounters[idx] >= pktByteLength)
                    {   // serve the packet
                        msg = (cMessage *)voq[idx]->pop();
                        voqCurrentSize[idx] -= pktByteLength;
                        deficitCounters[idx] -= pktByteLength;

                        // check whether the deficit counter value is enough for the HOL packet
                        if (!voq[idx]->isEmpty())
                        {
                            pktByteLength = PK(voq[idx]->front())->getByteLength();
                            if (deficitCounters[idx] >= pktByteLength)
                            {   // set the flag and the start queue index
                                continuation = true;
                                currentVoqIndex = idx;
                            }
                            else
                            {
                                currentVoqIndex = (idx + 1) % numFlows;
                            }
                        }
                        break;  // from the while loop
                    }
                }
                i++;
            } // end of while()
        }
    }   // end of DRR scheduling

    return (msg);   // just in case
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
#ifndef NDEBUG
        pktReqVector.record(packetRequested);
#endif
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
    unsigned long sumPktsConformed = 0;
    unsigned long sumPktsDropped = 0;

    for (int i=0; i < numFlows; i++)
    {
        std::stringstream ss_received, ss_conformed, ss_dropped, ss_sent, ss_throughput;
        ss_received << "packets received from flow[" << i << "]";
        ss_conformed << "packets conformed from flow[" << i << "]";
        ss_dropped << "packets dropped from flow[" << i << "]";
        ss_sent << "packets sent from flow[" << i << "]";
        ss_throughput << "bits/sec sent from flow[" << i << "]";
        recordScalar((ss_received.str()).c_str(), numPktsReceived[i]);
        recordScalar((ss_conformed.str()).c_str(), numPktsConformed[i]);
        recordScalar((ss_dropped.str()).c_str(), numPktsDropped[i]);
        recordScalar((ss_sent.str()).c_str(), numPktsSent[i]);
        recordScalar((ss_throughput.str()).c_str(), numBitsSent[i]/(simTime()-simulation.getWarmupPeriod()).dbl());
        sumPktsReceived += numPktsReceived[i];
        sumPktsConformed += numPktsConformed[i];
        sumPktsDropped += numPktsDropped[i];
    }
    recordScalar("overall packet conformed rate", sumPktsConformed/double(sumPktsReceived));
    recordScalar("overall packet loss rate", sumPktsDropped/double(sumPktsReceived));
}
