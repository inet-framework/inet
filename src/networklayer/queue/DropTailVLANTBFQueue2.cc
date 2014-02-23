//
// Copyright (C) 2014 Kyeong Soo (Joseph) Kim
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


#include "DropTailVLANTBFQueue2.h"

Define_Module(DropTailVLANTBFQueue2);

DropTailVLANTBFQueue2::DropTailVLANTBFQueue2()
{
}

DropTailVLANTBFQueue2::~DropTailVLANTBFQueue2()
{
    for (int i=0; i<numFlows; i++)
    {
        delete voq[i];
        cancelAndDelete(conformityTimer[i]);
    }
}

void DropTailVLANTBFQueue2::initialize()
{
    PassiveQueueBase::initialize();

    outGate = gate("out");

    // general
    // frameCapacity = par("frameCapacity");
    numFlows = par("numFlows");
//    bucketSize = par("bucketSize").longValue()*8LL; // in bit
//    meanRate = par("meanRate"); // in bps
//    mtu = par("mtu").longValue()*8; // in bit
//    peakRate = par("peakRate"); // in bps

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

    // Per-subscriber VOQs
    voqSize = par("voqSize").longValue();
    voq.assign(numFlows, (cQueue *)NULL);
    // meanBucketLength.assign(numFlows, bucketSize);
    // peakBucketLength.assign(numFlows, mtu);
    // lastTime.assign(numFlows, simTime());
    conformityFlag.assign(numFlows, false);
    conformityTimer.assign(numFlows, (cMessage *)NULL);
    for (int i=0; i<numFlows; i++)
    {
        char buf[32];
        sprintf(buf, "queue-%d", i);
        voq[i] = new cQueue(buf);
        conformityTimer[i] = new cMessage("Conformity Timer", i);   // message kind carries a voq index
    }

    // RR scheduler
    currentFlowIndex = 0;

    // statistic
    warmupFinished = false;
    numBitsSent.assign(numFlows, 0.0);
    numPktsReceived.assign(numFlows, 0);
    numPktsDropped.assign(numFlows, 0);
    numPktsUnshaped.assign(numFlows, 0);
    numPktsSent.assign(numFlows, 0);
}

void DropTailVLANTBFQueue2::handleMessage(cMessage *msg)
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

    if (msg->isSelfMessage())
    {   // Conformity Timer expires
        int flowIndex = msg->getKind();    // message kind carries a queue index
        conformityFlag[flowIndex] = true;

        // update TBF status
        // int pktLength = (check_and_cast<cPacket *>(voq[flowIndex]->front()))->getBitLength();
        // bool conformance = isConformed(flowIndex, pktLength);
        cPacket *holPkt = check_and_cast<cPacket *>(voq[flowIndex]->front());
        bool conformance = (tbm[flowIndex]->meterPacket(holPkt) == 0) ? true : false;   // result of metering; 0 for conformed and 1 for non-conformed packet
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
                    numBitsSent[currentFlowIndex] += pkt->getBitLength();
                    numPktsSent[currentFlowIndex]++;
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
        int flowIndex = classifier->classifyPacket(msg);
        cQueue *queue = voq[flowIndex];
        if (warmupFinished == true)
        {
            numPktsReceived[flowIndex]++;
        }
        int pktLength = (check_and_cast<cPacket *>(msg))->getBitLength();
// DEBUG
        ASSERT(pktLength > 0);
// DEBUG
        if (queue->isEmpty())
        {
            if (tbm[flowIndex]->meterPacket(msg) == 0)
            {   // packet is conformed
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
                    currentFlowIndex = flowIndex;
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
            {   // packet is not conformed
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
                    triggerConformityTimer(flowIndex, pktLength);
                    conformityFlag[flowIndex] = false;
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

bool DropTailVLANTBFQueue2::enqueue(cMessage *msg)
{
    int flowIndex = classifier->classifyPacket(msg);
    int pktByteLength = PK(msg)->getByteLength();
    // cQueue *queue = voq[flowIndex];

    if (voqCurrentSize[flowIndex] + pktByteLength > voqSize)
    // if (frameCapacity && queue->length() >= frameCapacity)
    {
        EV << "VOQ[" << flowIndex << "] full, dropping packet.\n";
        delete msg;
        return true;
    }
    else
    {
        voq[flowIndex]->insert(msg);
        voqCurrentSize[flowIndex] += pktByteLength;
        // queue->insert(msg);
        return false;
    }
}

cMessage *DropTailVLANTBFQueue2::dequeue()
{
    bool found = false;
    int startQueueIndex = (currentFlowIndex + 1) % numFlows;  // search from the next queue for a frame to transmit
    for (int i = 0; i < numFlows; i++)
    {
       if (conformityFlag[(i+startQueueIndex)%numFlows])
       {
           currentFlowIndex = (i+startQueueIndex)%numFlows;
           found = true;
           break;
       }
    }

    if (found == false)
    {
        // TO DO: further processing?
        return NULL;
    }

    cMessage *msg = (cMessage *)voq[currentFlowIndex]->pop();

    // TO DO: update statistics

    // conformity processing for the HOL frame
    if (voq[currentFlowIndex]->isEmpty() == false)
    {
        // int pktLength = (check_and_cast<cPacket *>(voq[currentFlowIndex]->front()))->getBitLength();
        cPacket *holPkt = check_and_cast<cPacket *>(voq[currentFlowIndex]->front());
        int pktLength = holPkt->getBitLength();
        if (tbm[currentFlowIndex]->meterPacket(holPkt) == 0)
        // if (isConformed(currentFlowIndex, pktLength))
        {
            conformityFlag[currentFlowIndex] = true;
        }
        else
        {
            conformityFlag[currentFlowIndex] = false;
            triggerConformityTimer(currentFlowIndex, pktLength);
        }
    }
    else
    {
        conformityFlag[currentFlowIndex] = false;
    }

    // return a packet from the scheduled queue for transmission
    return (msg);
}

void DropTailVLANTBFQueue2::sendOut(cMessage *msg)
{
    send(msg, outGate);
}

void DropTailVLANTBFQueue2::requestPacket()
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
            numBitsSent[currentFlowIndex] += (check_and_cast<cPacket *>(msg))->getBitLength();
            numPktsSent[currentFlowIndex]++;
        }
        sendOut(msg);
    }
}

// bool DropTailVLANTBFQueue2::isConformed(int flowIndex, int pktLength)
// {
//     Enter_Method("isConformed()");

// // DEBUG
//     EV << "Last Time = " << lastTime[flowIndex] << endl;
//     EV << "Current Time = " << simTime() << endl;
//     EV << "Packet Length = " << pktLength << endl;
// // DEBUG

//     // update states
//     simtime_t now = simTime();
//     //unsigned long long meanTemp = meanBucketLength[flowIndex] + (unsigned long long)(meanRate*(now - lastTime[flowIndex]).dbl() + 0.5);
//     unsigned long long meanTemp = meanBucketLength[flowIndex] + (unsigned long long)ceil(meanRate*(now - lastTime[flowIndex]).dbl());
//     meanBucketLength[flowIndex] = (long long)((meanTemp > bucketSize) ? bucketSize : meanTemp);
//     //unsigned long long peakTemp = peakBucketLength[flowIndex] + (unsigned long long)(peakRate*(now - lastTime[flowIndex]).dbl() + 0.5);
//     unsigned long long peakTemp = peakBucketLength[flowIndex] + (unsigned long long)ceil(peakRate*(now - lastTime[flowIndex]).dbl());
//     peakBucketLength[flowIndex] = int((peakTemp > mtu) ? mtu : peakTemp);
//     lastTime[flowIndex] = now;

//     if (pktLength <= meanBucketLength[flowIndex])
//     {
//         if  (pktLength <= peakBucketLength[flowIndex])
//         {
//             meanBucketLength[flowIndex] -= pktLength;
//             peakBucketLength[flowIndex] -= pktLength;
//             return true;
//         }
//     }
//     return false;
// }

// trigger TBF conformity timer for the HOL frame in the queue,
// indicating that enough tokens will be available for its transmission
void DropTailVLANTBFQueue2::triggerConformityTimer(int flowIndex, int pktLength)
{
    Enter_Method("triggerConformityCounter()");

    double meanDelay = (pktLength - tbm[flowIndex]->getMeanBucketLength()) / tbm[flowIndex]->getMeanRate();
    double peakDelay = (pktLength - tbm[flowIndex]->getPeakBucketLength()) / tbm[flowIndex]->getPeakRate();

// DEBUG
    EV << "Packet Length = " << pktLength << endl;
    dumpTbfStatus(flowIndex);
    EV << "Delay for Mean TBF = " << meanDelay << endl;
    EV << "Delay for Peak TBF = " << peakDelay << endl;
    EV << "Current Time = " << simTime() << endl;
    EV << "Counter Expiration Time = " << simTime() + std::max(meanDelay, peakDelay) << endl;
// DEBUG

    scheduleAt(simTime() + std::max(meanDelay, peakDelay), conformityTimer[flowIndex]);
}

void DropTailVLANTBFQueue2::dumpTbfStatus(int flowIndex)
{
    EV << "Last Time = " << tbm[flowIndex]->getLastTime() << endl;
    EV << "Current Time = " << simTime() << endl;
    EV << "Token bucket for mean rate/burst control " << endl;
    EV << "- Bucket size [bit]: " << tbm[flowIndex]->getBucketSize() << endl;
    EV << "- Mean rate [bps]: " << tbm[flowIndex]->getMeanRate() << endl;
    EV << "- Bucket length [bit]: " << tbm[flowIndex]->getMeanBucketLength() << endl;
    EV << "Token bucket for peak rate/MTU control " << endl;
    EV << "- MTU [bit]: " << tbm[flowIndex]->getMtu() << endl;
    EV << "- Peak rate [bps]: " << tbm[flowIndex]->getPeakRate() << endl;
    EV << "- Bucket length [bit]: " << tbm[flowIndex]->getPeakBucketLength() << endl;
}

void DropTailVLANTBFQueue2::finish()
{
    unsigned long sumPktsReceived = 0;
    unsigned long sumPktsDropped = 0;
    unsigned long sumPktsShaped = 0;
    unsigned long sumPktsUnshaped = 0;

    for (int i=0; i < numFlows; i++)
    {
        std::stringstream ss_received, ss_dropped, ss_shaped, ss_sent, ss_throughput;
        ss_received << "packets received from flow[" << i << "]";
        ss_dropped << "packets dropped from flow[" << i << "]";
        ss_shaped << "packets shaped from flow[" << i << "]";
        ss_sent << "packets sent from flow[" << i << "]";
        ss_throughput << "bits/sec sent from flow[" << i << "]";
        recordScalar((ss_received.str()).c_str(), numPktsReceived[i]);
        recordScalar((ss_dropped.str()).c_str(), numPktsDropped[i]);
        recordScalar((ss_shaped.str()).c_str(), numPktsReceived[i]-numPktsUnshaped[i]);
        recordScalar((ss_sent.str()).c_str(), numPktsSent[i]);
        recordScalar((ss_throughput.str()).c_str(), numBitsSent[i]/(simTime()-simulation.getWarmupPeriod()).dbl());
        sumPktsReceived += numPktsReceived[i];
        sumPktsDropped += numPktsDropped[i];
        sumPktsShaped += numPktsReceived[i] - numPktsUnshaped[i];
    }
    recordScalar("overall packet loss rate", sumPktsDropped/double(sumPktsReceived));
    recordScalar("overall packet shaped rate", sumPktsShaped/double(sumPktsReceived));
}
