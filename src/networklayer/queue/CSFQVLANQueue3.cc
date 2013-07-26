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


#include "CSFQVLANQueue3.h"

Define_Module(CSFQVLANQueue3);

void CSFQVLANQueue3::initialize(int stage)
{
    CSFQVLANQueue::initialize(stage);

    if (stage == 1)
    {   // the following should be initialized in the 2nd stage (stage==1)
        // because token bucket meters are not initialized in the 1st stage yet.

        // FIFO queue
        queueThreshold = par("queueThreshold").longValue(); // in byte

        // CSFQ++: System-wide variables
        fairShareRate = excessBW;        
        kalpha = KALPHA;
    }   // end of if () for stage checking
}

void CSFQVLANQueue3::handleMessage(cMessage *msg)
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
            estimateRate(flowIndex, pktLength, simTime(), 0);

            // update excess BW
            double sum = 0.0;
            for (int i = 0; i < numFlows; i++)
            {
                sum += flowRate[i][0];    // sum of conformed rates
            }
            excessBW = std::max(linkRate - sum, 0.0);
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
            double rate = estimateRate(flowIndex, pktLength, simTime(), 1);
            if (fairShareRate * weight[flowIndex] / rate < dblrand())
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

                        // decrease fairShareRate; the number of times it is decreased
                        // during an interval of length k is bounded by kalpha.
                        // This is to avoid overcorrection.
                        if (kalpha-- >= 0)
                        {
                            fairShareRate *= 0.99;
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

// estimate the link's (normalized) fair share rate (alpha)
// - pktLength: packet length in bit
// - rate: estimated normalized flow rate
// - arrvTime: packet arrival time
// - dropped: flag indicating whether the packet is dropped or not
void CSFQVLANQueue3::estimateAlpha(int pktLength, double rate, simtime_t arrvTime, bool dropped)
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
        return; // TODO: check this!
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
        {   // previously uncongested
            if (currentQueueSize >= queueThreshold)
            {   // becomes congested (per both rate estimation and queue status)
                congested = true;
                startTime = arrvTime;
                kalpha = KALPHA;
                if (fairShareRate == 0.0)
                {
                    fairShareRate = std::min(rate, excessBW);   // initialize
                }
            }
            else
            {   // remains uncongested (per previous congestion and queue statuses)
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
            maxRate = 0.0;
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

