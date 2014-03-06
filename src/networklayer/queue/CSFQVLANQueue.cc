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


#include "CSFQVLANQueue.h"

Define_Module(CSFQVLANQueue);

CSFQVLANQueue::CSFQVLANQueue()
{
}

CSFQVLANQueue::~CSFQVLANQueue()
{
#ifndef NDEBUG
    for (int i = 0; i <= numFlows; i++) {
        delete estRateVectors[i];
    }
#endif
}

void CSFQVLANQueue::initialize(int stage)
{
    if (stage == 0)
    {   // the following should be initialized in the 1st stage (stage==0);
        // otherwise VLAN ID initialization is not working properly.

        PassiveQueueBase::initialize();

        outGate = gate("out");

        // general
        numFlows = par("numFlows").longValue(); // also the number of subscribers
        linkRate = par("linkRate").doubleValue();

        // VLAN classifier
        const char *classifierClass = par("classifierClass").stringValue();
        classifier = check_and_cast<IQoSClassifier *>(createOne(classifierClass));
        classifier->setMaxNumQueues(numFlows);
        const char *vids = par("vids").stringValue();
        classifier->initialize(vids);

        // debugging
#ifndef NDEBUG
        excessBWVector.setName("excess bandwidth");
        fairShareRateVector.setName("fair share rate (alpha)");
        pktReqVector.setName("packets requested");
        queueLengthVector.setName("FIFO queue length (packets)");
        queueSizeVector.setName("FIFO queue size (bytes)");
        rateTotalVector.setName("aggregate rate of non-conformed packets");
        rateEnqueuedVector.setName("aggregate rate of enqueued non-conformed packets");
        for (int i = 0; i < numFlows; i++) {
            std::stringstream vname;
            vname << "non-conformed rate for flow[" << i << "] (bit/sec)";
            cOutVector *vector = new cOutVector((vname.str()).c_str());
            estRateVectors.push_back(vector);
        }
        cOutVector *vector = new cOutVector("conformed rate (bit/sec)");
        estRateVectors.push_back(vector);
#endif
    }
    if (stage == 1)
    {   // the following should be initialized in the 2nd stage (stage==1)
        // because token bucket meters are not initialized in the 1st stage yet.

        // Token bucket meters
        tbm.assign(numFlows, (BasicTokenBucketMeter *)NULL);
        cModule *mac = getParentModule();
        for (int i=0; i<numFlows; i++)
        {
            cModule *meter = mac->getSubmodule("meter", i);
            tbm[i] = check_and_cast<BasicTokenBucketMeter *>(meter);
        }

        // FIFO queue
        fifo.setName("FIFO queue");
        queueSize = par("queueSize").longValue();           // in byte
        currentQueueSize = 0;                               // in byte

        // CSFQ++: System-wide variables
        K = par("K").doubleValue();
        double tmp = par("K_scale").doubleValue();
        K_scale = (K_scale == 0.0) ? 1.0/numFlows : tmp; // default value (when the parameter value is 0) is 1.0/numFlows.
        K_alpha = par("K_alpha").doubleValue();
        excessBW = linkRate;
        fairShareRate = excessBW;
        rateTotal = 0.0;
        rateEnqueued = 0.0;
        maxRate = 0.0;
        lastArv = 0.0;
        startTime = 0.0;
        sumBitsTotal = 0;
        sumBitsEnqueued = 0;
        congested = false;

        // CSFQ++: Flow-specific variables
        weight.assign(numFlows, 0.0);
        sumBits.assign(numFlows, 0);
        flowRate.assign(numFlows+1, 0.0);   // last index (i.e., numFlows) for combined conformant flow
        prevTime.assign(numFlows+1, 0.0);   // last index (i.e., numFlows) for combined conformant flow

        double minTGR = tbm[0]->getMeanRate();
        for (int i = 1; i < numFlows; i++)
        {
            if (tbm[i]->getMeanRate() < minTGR)
            {
                minTGR = tbm[i]->getMeanRate();
            }
        }
        for (int i = 0; i < numFlows; i++) {
            weight[i] = tbm[i]->getMeanRate() / minTGR;
        }

        // statistics
        warmupFinished = false;
        numBitsSent.assign(numFlows, 0.0);
        numPktsReceived.assign(numFlows, 0);
        numPktsDropped.assign(numFlows, 0);
        numPktsConformed.assign(numFlows, 0);
        numPktsSent.assign(numFlows, 0);
    }   // end of if () for stage checking
}

void CSFQVLANQueue::handleMessage(cMessage *msg)
{
    if (warmupFinished == false)
    {   // start statistics gathering once the warm-up period has passed.
        if (simTime() >= simulation.getWarmupPeriod())
        {
            warmupFinished = true;
        }
    }

    if (!msg->isSelfMessage())
    {   // a frame arrives
        int flowIndex = classifier->classifyPacket(msg);
        int pktLength = PK(msg)->getBitLength();
// DEBUG
        ASSERT(pktLength > 0);
// DEBUG
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
            estimateRate(numFlows, pktLength, simTime());   // index of numFlows for conformed packets

            // update excess BW
            excessBW = std::max(linkRate - flowRate[numFlows], 0.0);
#ifndef NDEBUG
            excessBWVector.record(excessBW);
#endif
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
                bool dropped = enqueue(msg);
                if (dropped)
                {   // buffer overflow
                    if (warmupFinished == true)
                    {
                        numPktsDropped[flowIndex]++;
                    }
                }
            }
        }
        else
        {   // frame is not conformed
            double rate = estimateRate(flowIndex, pktLength, simTime());
#ifndef NDEBUG
            double rv = dblrand();
            if (fairShareRate * weight[flowIndex] / rate < rv)
#else
            if (fairShareRate * weight[flowIndex] / rate < dblrand())
#endif
            {   // probabilistically drop the frame
#ifndef NDEBUG
                double normalizedRate = rate / weight[flowIndex];
                estimateAlpha(pktLength, normalizedRate, simTime(), true);
#else
                estimateAlpha(pktLength, rate/weight[flowIndex], simTime(), true);
#endif
                delete msg;
                if (warmupFinished == true)
                {
                    numPktsDropped[flowIndex]++;
                }
            }
            else
            {   // send or enqueue the frame
                if (packetRequested > 0)
                {   // send the frame
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
#ifndef NDEBUG
                    double normalizedRate = rate / weight[flowIndex];
                    estimateAlpha(pktLength, normalizedRate, simTime(), false);
#else
                    estimateAlpha(pktLength, rate/weight[flowIndex], simTime(), false);
#endif
                }
                else
                {   // enqueue the frame
                    bool dropped = enqueue(msg);

                    // we call estimateAlpha() with dropped=false even when the packet is dropped,
                    // as this packet would have been enqueued by CSFQ
#ifndef NDEBUG
                    double normalizedRate = rate / weight[flowIndex];
                    estimateAlpha(pktLength, normalizedRate, simTime(), false);
#else
                    estimateAlpha(pktLength, rate/weight[flowIndex], simTime(), false);
#endif
                    if (dropped)
                    {   // buffer overflow
                        if (warmupFinished == true)
                        {
                            numPktsDropped[flowIndex]++;
                        }
                    }
                }
            }

#ifndef NDEBUG
            fairShareRateVector.record(fairShareRate);
            rateTotalVector.record(rateTotal);
            rateEnqueuedVector.record(rateEnqueued);
#endif
        }

        if (ev.isGUI())
        {
            char buf[40];
            sprintf(buf, "q rcvd: %lu\nq dropped: %lu", numPktsReceived[flowIndex], numPktsDropped[flowIndex]);
            getDisplayString().setTagArg("t", 0, buf);
        }
    }
    else
    {   // self message is not expected; something wrong
        error("%s::handleMessage: Unexpected self message", getFullPath().c_str());
    }
}

bool CSFQVLANQueue::enqueue(cMessage *msg)
{
    int pktByteLength = PK(msg)->getByteLength();

    if (currentQueueSize + pktByteLength > queueSize)
    {
        EV << "FIFO queue full, dropping packet.\n";
        delete msg;
        return true;
    }
    else
    {
        fifo.insert(msg);
        currentQueueSize += pktByteLength;
#ifndef NDEBUG
    queueLengthVector.record(fifo.getLength());
    queueSizeVector.record(currentQueueSize);
#endif
        return false;
    }
}

cMessage *CSFQVLANQueue::dequeue()
{
    if (fifo.empty())
    {
        return NULL;
    }
    cMessage *msg = (cMessage *)fifo.pop();
    currentQueueSize -= PK(msg)->getByteLength();
#ifndef NDEBUG
    queueLengthVector.record(fifo.getLength());
    queueSizeVector.record(currentQueueSize);
#endif
    return (msg);
}

void CSFQVLANQueue::sendOut(cMessage *msg)
{
    send(msg, outGate);
}

void CSFQVLANQueue::requestPacket()
{
    Enter_Method("requestPacket()");

    cMessage *msg = dequeue();
    if (msg == NULL)
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
            int flowIndex = classifier->classifyPacket(msg);
            numBitsSent[flowIndex] += PK(msg)->getBitLength();
            numPktsSent[flowIndex]++;
        }
        sendOut(msg);
    }
}

// compute estimated flow rate by using exponential averaging
// - flowIndex from 0 to numFlows-1: flow index of non-conformed packet
// - flowIndex of numFlows: combined flow of conformed packets
double CSFQVLANQueue::estimateRate(int flowIndex, int pktLength, simtime_t arrvTime)
{
    double T = (arrvTime - prevTime[flowIndex]).dbl();   // packet interarrival

    if (T == 0.0)
    {   // multiple packets arrive simultaneously
        sumBits[flowIndex] += pktLength;
        if (flowRate[flowIndex])
        {
#ifndef NDEBUG
            estRateVectors[flowIndex]->record(flowRate[flowIndex]);
#endif
            return flowRate[flowIndex];
        }
        else
        {   // this is the first packet; just initialize the rate
#ifndef NDEBUG
            estRateVectors[flowIndex]->record(rateEnqueued / 2);
#endif
            return (flowRate[flowIndex] = rateEnqueued / 2);
        }
    }
    else
    {
        pktLength += sumBits[flowIndex];
        sumBits[flowIndex] = 0;
    }

    prevTime[flowIndex] = arrvTime;

    // use a different averaging constant for conformant and non-conformant flow rate estimation
    // double w = exp(-T/K);
    double w = (flowIndex < numFlows ? exp(-T/K) : exp(-T/(K_scale*K)));

    flowRate[flowIndex] = (1 - w)*pktLength/T + w*flowRate[flowIndex];
#ifndef NDEBUG
            estRateVectors[flowIndex]->record(flowRate[flowIndex]);
#endif
    return flowRate[flowIndex];
}

// estimate the link's (normalized) fair share rate (alpha)
// - pktLength: packet length in bit
// - rate: estimated normalized flow rate
// - arrvTime: packet arrival time
// - dropped: flag indicating whether the packet is dropped or not
void CSFQVLANQueue::estimateAlpha(int pktLength, double rate, simtime_t arrvTime, bool dropped)
{
    double T = (arrvTime - lastArv).dbl();   // packet interarrival
    double k = K_alpha;    // the window size (i.e., K_c)

    // set lastArv to the arrival time of the first packet
    if (lastArv == 0.0)
    {
        lastArv = arrvTime;
    }

    // account for packets received simultaneously
    sumBitsTotal += pktLength;
    if (dropped == false)
    {   // this packet was enqueued for transmission
        sumBitsEnqueued += pktLength;
    }
    if (arrvTime == lastArv)
    {
        return;
    }

    // estimate aggregate arrival (rateTotal) and enqueued (rateEnqueued) rates
    double w = exp(-T / K_alpha);
    rateTotal = (1 - w)*sumBitsTotal/T + w*rateTotal;
    rateEnqueued = (1 - w)*sumBitsEnqueued/T + w*rateEnqueued;
    lastArv = arrvTime;
    sumBitsTotal = sumBitsEnqueued = 0;

    // update alpha
    if (rateTotal >= excessBW)
    {   // link is congested per rate estimation
        if (!congested)
        {   // becomes congested
            congested = true;
            startTime = arrvTime;
            if (fairShareRate == 0.0)
            {
                fairShareRate = std::min(rate, excessBW);   // initialize
            }
        }
        else
        {   // remains congested
            if (arrvTime > startTime + k)
            {
                startTime = arrvTime;
                fairShareRate *= excessBW / rateEnqueued;
                fairShareRate = std::min(fairShareRate, excessBW);

                // TODO: verify the following re-initialization
                if (fairShareRate == 0.0)
                {
                    fairShareRate = std::min(rate, excessBW);   // initialize
                }

                // // TODO: check the validity of this reset
                // congested = false;
            }
        }
    }
    else
    {   // link is not congested per rate estimation
        if (congested)
        {   // becomes uncongested
            congested = false;
            startTime = arrvTime;
            maxRate = 0;
        }
        else
        {   // remains uncongested
            if (arrvTime < startTime + k)
            {
                maxRate = std::max(maxRate, rate);
            }
            else
            {
                fairShareRate = maxRate;
                startTime = arrvTime;
                maxRate = 0.0;
            }
        }
    }
}

void CSFQVLANQueue::finish()
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
