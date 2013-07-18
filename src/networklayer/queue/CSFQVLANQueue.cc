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
#ifdef NDEBUG
#else
    for (int i = 0; i < numFlows; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            delete estRateVectors[i][j];
        }
    }
#endif
}

void CSFQVLANQueue::initialize()
{
    PassiveQueueBase::initialize();

    outGate = gate("out");

    // general
    numFlows = par("numFlows"); // also the number of subscribers

    // VLAN classifier
    const char *classifierClass = par("classifierClass");
    classifier = check_and_cast<IQoSClassifier *>(createOne(classifierClass));
    classifier->setMaxNumQueues(numFlows);
    const char *vids = par("vids");
    classifier->initialize(vids);

    // Token bucket meters
    tbm.assign(numFlows, (BasicTokenBucketMeter *)NULL);
    cModule *mac = getParentModule();
    for (int i=0; i<numFlows; i++)
    {
        cModule *meter = mac->getSubmodule("meter", i);
        tbm[i] = check_and_cast<BasicTokenBucketMeter *>(meter);
    }

    // FIFO queue
    queueSize = par("queueSize");   // in bit
    queueThreshold = par("queueThreshold"); // in bit
    currentQueueSize = 0;
    fifo.setName("FIFO queue");

    // CSFQ++
    double K = par("K").doubleValue();
    flowState.resize(numFlows);
    for (int i = 0; i < numFlows; i++) {
        flowState[i].weight = tbm[i]->getMeanRate();
        flowState[i].K = K;
        flowState[i].numArv_ = flowState[i].numDpt_ = flowState[i].numDropped_ = 0;
        flowState[i].sumPktSize = 0;
        for (int j = 0; j < 2; j++)
        {
            flowState[i].estRate[j]= 0.0;
            flowState[i].prevTime[j]= 0.0;
        }
#ifdef SRCID
        hashid_[i]              = 0;
#endif
    }

    csfq.linkRate = par("linkRate").doubleValue();
    csfq.K_alpha = par("K_alpha").doubleValue();
    csfq.alpha = csfq.tmpAlpha = 0.;
    csfq.kalpha = KALPHA;

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

    csfq.lastArv = csfq.startTime = 0.;
    csfq.rateAlpha = csfq.rateTotal = 0.;
    csfq.pktLength = csfq.pktLengthEnqueued = 0;
    csfq.congested = 1;

#ifdef PENALTY_BOX
    bind("kLink_", &kDropped_);
#endif

    // statistics
    warmupFinished = false;
    numBitsSent.assign(numFlows, 0.0);
    numPktsReceived.assign(numFlows, 0);
    numPktsDropped.assign(numFlows, 0);
    numPktsConformed.assign(numFlows, 0);
    numPktsSent.assign(numFlows, 0);

    // debugging
#ifdef NDEBUG
#else
    for (int i=0; i < numFlows; i++)
    {
        OutVectorVector estRateVector;
        for (int j=0; j < 2; j++)
        {
            std::stringstream vname;
            vname << (j == 0 ? "conformed" : "non-conformed") << " rate for flow[" << i << "] (bit/sec)";
            cOutVector *vector = new cOutVector((vname.str()).c_str());
            estRateVector.push_back(vector);
        }
        estRateVectors.push_back(estRateVector);
    }
#endif
}

void CSFQVLANQueue::handleMessage(cMessage *msg)
{
    if (warmupFinished == false)
    {   // start statistics gathering once the warm-up period has passed.
        if (simTime() >= simulation.getWarmupPeriod()) {
            warmupFinished = true;
//            for (int i = 0; i < numFlows; i++)
//            {
//                numPktsReceived[i] = queues[i]->getLength();   // take into account the frames/packets already in queues
//            }
        }
    }

    if (!msg->isSelfMessage())
    {   // a frame arrives
        int flowIndex = classifier->classifyPacket(msg);
        if (warmupFinished == true)
        {
            numPktsReceived[flowIndex]++;
        }
        int pktLength = PK(msg)->getBitLength();
// DEBUG
        ASSERT(pktLength > 0);
// DEBUG
        if (tbm[flowIndex]->meterPacket(msg) == 0)
        {   // frame is conformed
            if (warmupFinished == true)
            {
                numPktsConformed[flowIndex]++;
            }
            estimateRate(flowIndex, pktLength, simTime(), 0);
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
            }
        }
        else
        {   // frame is not conformed
//            conformedRate[flowIndex] = tbm[flowIndex]->getMeanRate(); // TODO: need skip?
            double estRate = estimateRate(flowIndex, pktLength, simTime(), 1);
//            if (std::max(0.0, 1.0 - csfq.alpha_ * flowState[flowIndex].weight / estRate) > dblrand())
            if (csfq.alpha * flowState[flowIndex].weight / estRate < dblrand())
            {   // drop the frame
                estimateAlpha(pktLength, estRate, simTime(), 1);
                delete msg;
                if (warmupFinished == true)
                {
                    numPktsDropped[flowIndex]++;
                }
            }
            else
            {
                estimateAlpha(pktLength, estRate, simTime(), 0);
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
    }
    else
    {   // self message is not expected; something wrong
        error("%s::handleMessage: Unexpected self message", getFullPath().c_str());
    }
}

bool CSFQVLANQueue::enqueue(cMessage *msg)
{
    int pktLength = PK(msg)->getBitLength();

    if (currentQueueSize + pktLength > queueSize)
    {
        EV << "FIFO queue full, dropping packet.\n";
        delete msg;
        return true;
    }
    else
    {
        fifo.insert(msg);
        currentQueueSize += pktLength;
        return false;
    }
}

cMessage *CSFQVLANQueue::dequeue()
{
    if (fifo.empty())
    {
        return NULL;
    }
    return ((cMessage *)fifo.pop());
}

void CSFQVLANQueue::sendOut(cMessage *msg)
{
    send(msg, outGate);
}

void CSFQVLANQueue::requestPacket()
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

// compute estimated flow rate by using exponential averaging
// color: result of metering -> 0 for conformed and 1 for non-conformed packets
double CSFQVLANQueue::estimateRate(int flowIndex, int pktLength, simtime_t arrvTime, int color)
{
    double T = (arrvTime - flowState[flowIndex].prevTime[color]).dbl(); // interarrival time of packet
    double K = flowState[flowIndex].K;

    if (T == 0.0)
    {   // multiple packets arrive simultaneously
        flowState[flowIndex].sumPktSize += pktLength;
        if (flowState[flowIndex].estRate[color])
        {
#if defined NDEBUG
#else
            estRateVectors[flowIndex][color]->record(flowState[flowIndex].estRate[color]);
#endif
            return flowState[flowIndex].estRate[color];
        }
        else
        {   // this is the first packet; just initialize the rate
#if defined NDEBUG
#else
            estRateVectors[flowIndex][color]->record(csfq.rateAlpha / 2);
#endif
            return (flowState[flowIndex].estRate[color] = csfq.rateAlpha / 2);
        }
    }
    else
    {
        pktLength += flowState[flowIndex].sumPktSize;
        flowState[flowIndex].sumPktSize = 0;
    }

    flowState[flowIndex].prevTime[color] = arrvTime;
    flowState[flowIndex].estRate[color] = (1. - exp(-T/K))*pktLength/T + exp(-T/K) * flowState[flowIndex].estRate[color];
#if defined NDEBUG
#else
            estRateVectors[flowIndex][color]->record(flowState[flowIndex].estRate[color]);
#endif
    return flowState[flowIndex].estRate[color];
}

// estimate the link's alpha parameter
void CSFQVLANQueue::estimateAlpha(int pktLength, double arrvRate, simtime_t arrvTime, int dropped)
{
    double T = (arrvTime - csfq.lastArv).dbl();
    double k = csfq.K_alpha / 1000000.;

    // set lastArv_ to the arrival time of the first packet
    if (csfq.lastArv == 0.)
    {
        csfq.lastArv = arrvTime;
    }

    // account for packets received simultaneously
    csfq.pktLength += pktLength;
    if (dropped == false)
    {   // this packet was enqueued for transmission
        csfq.pktLengthEnqueued += pktLength;
    }
    if (arrvTime == csfq.lastArv)
    {
        return;
    }

    // estimate the aggregate arrival rate (rateTotal_) and the
    // aggregate forwarded (rateAlpha) rates
    double w = exp(-T / csfq.K_alpha);
    csfq.rateAlpha = (1 - w) * (double) csfq.pktLengthEnqueued / T + w * csfq.rateAlpha;
    csfq.rateTotal = (1 - w) * (double) csfq.pktLength / T + w * csfq.rateTotal;
    csfq.lastArv = arrvTime;
    csfq.pktLength = csfq.pktLengthEnqueued = 0;

    // compute the initial value of alpha
    if (csfq.alpha == 0.)
    {
        if (currentQueueSize < queueThreshold)
        {   // not congested
            if (csfq.tmpAlpha < arrvRate)
            {   // update the largest rate
                csfq.tmpAlpha = arrvRate;
            }
            return;
        }
        if (csfq.alpha < csfq.tmpAlpha)
        {
            csfq.alpha = csfq.tmpAlpha;
        }
        if (csfq.alpha == 0.)
        {
            csfq.alpha = csfq.linkRate / 2.;  // arbitrary initialization
        }
        csfq.tmpAlpha = 0.;
    }

    // update alpha
    double sum = 0.0;
    for (int i = 0; i < numFlows; i++)
    {
        sum += flowState[i].estRate[0];    // sum of conformed rates
    }
    double excessBW = csfq.linkRate - sum;    // excess bandwidth
    if (csfq.rateTotal >= excessBW)
    {   // link congested
        if (!csfq.congested)
        {
            csfq.congested = true;
            csfq.startTime = arrvTime;
            csfq.kalpha = KALPHA;
        }
        else
        {
            if (arrvTime < csfq.startTime + k)
            {
                if (csfq.tmpAlpha < arrvRate)
                {
                    csfq.tmpAlpha = arrvRate;
                }
            }
            else
            {
                csfq.alpha *= excessBW / csfq.rateAlpha;
                csfq.startTime = arrvTime;
            }
            if (csfq.linkRate < csfq.alpha)
            {
                csfq.alpha = csfq.linkRate;
            }
        }
    }
    else
    {   // link not congested
        if (csfq.congested)
        {
            csfq.congested = false;
            csfq.startTime = arrvTime;
            csfq.tmpAlpha = 0;
        }
        else
        {
            if (arrvTime < csfq.startTime + k)
            {
                if (csfq.tmpAlpha < arrvRate)
                {
                    csfq.tmpAlpha = arrvRate;
                }
            }
            else
            {
                csfq.alpha = csfq.tmpAlpha;
                csfq.startTime = arrvTime;
                if (currentQueueSize < queueThreshold)
                {
                    csfq.alpha = 0.;
                }
                else
                {
                    csfq.tmpAlpha = 0.;
                }
            }
        }
    }
#ifdef CSFQ_LOG
    EV << id_ << " " << arrvTime << " " << csfq.rateAlpha << " " << csfq.rateTotal;
#endif
}

void CSFQVLANQueue::finish()
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
    recordScalar("overall packet loss rate", sumPktsDropped/double(sumPktsReceived));
    recordScalar("overall packet shaped rate", sumPktsNonconformed/double(sumPktsReceived));
}
