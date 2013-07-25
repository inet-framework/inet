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


#include "CSFQVLANQueue2.h"

Define_Module(CSFQVLANQueue2);

void CSFQVLANQueue2::initialize(int stage)
{
    CSFQVLANQueue::initialize(stage);

    if (stage == 1)
    {   // the following should be initialized in the 2nd stage (stage==1)
        // because token bucket meters are not initialized in the 1st stage yet.

        // FIFO queue
        queueThreshold = par("queueThreshold").longValue(); // in byte

        // CSFQ++: System-wide variables
        fairShareRate = 0.0;
        kalpha = KALPHA;
    }   // end of if () for stage checking
}

void CSFQVLANQueue2::handleMessage(cMessage *msg)
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
            if ( (fairShareRate > 0.0) && (fairShareRate * weight[flowIndex] / rate < dblrand()) )
            {   // probabilistically drop the frame
                // note that we skip packet dropping when fairShareRate == 0.0;
                // fairShareRate is set to 0 when the queue size is below the threshold
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

                // we call estimateAlpha() with dropped=false even when the packet is dropped,
                // as this packet would have been enqueued by CSFQ
#ifndef NDEBUG
                double normalizedRate = rate / weight[flowIndex];
                estimateAlpha(pktLength, normalizedRate, simTime(), false);
#else
                estimateAlpha(pktLength, rate/weight[flowIndex], simTime(), false);
#endif
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
                }
                else
                {   // enqueue the frame
                    bool dropped = enqueue(msg);
                    if (dropped)
                    {   // buffer overflow
                        if (warmupFinished == true)
                        {
                            numPktsDropped[flowIndex]++;
                        }

                        // buffer-related amendment #1:
                        // decrease the fairShareRate every time the buffer overflows;
                        // the number of times it is decreased during an interval of length k
                        // is bounded by kalpha to avoid overcorrection.
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
            sprintf(buf, "q rcvd: %d\nq dropped: %d", numPktsReceived[flowIndex], numPktsDropped[flowIndex]);
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
void CSFQVLANQueue2::estimateAlpha(int pktLength, double rate, simtime_t arrvTime, bool dropped)
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

    // compute the initial value of fair share rate (alpha).
    // note that whenever fair share rate is reset to 0, packet dropping is skipped.
    if (fairShareRate == 0.0)
    {
        if (currentQueueSize < queueThreshold)
        {   // not congested
            maxRate = std::max(maxRate, rate);
            return;
        }
        fairShareRate = std::max(fairShareRate, maxRate);
        if (fairShareRate == 0.0)
        {
            // TODO: justify below
            fairShareRate = linkRate / 2.;  // arbitrary initialization
            // fairShareRate = excessBW / 2.;  // arbitrary initialization
        }
        maxRate = 0.0;
    }

    // update alpha
    if (rateTotal >= excessBW)
    {   // link is congested
        if (!congested)
        {
            congested = true;
            startTime = arrvTime;
            kalpha = KALPHA;
//            if (fairShareRate == 0.0)
//            {
//                fairShareRate = std::min(rate, excessBW);   // initialize
//            }
        }
        else
        {
            if (arrvTime < startTime + k)
            {
                return;
            }
//            {
//                maxRate = std::max(maxRate, rate);  // TODO: check this
//            }
//            else
            startTime = arrvTime;
            fairShareRate *= excessBW / rateEnqueued;
            fairShareRate = std::min(fairShareRate, excessBW);
//                // TODO: check the validity of this reset
//                congested = false;
        }
    }
    else
    {   // link is not congested
        if (congested)
        {
            congested = false;
            startTime = arrvTime;
            maxRate = 0.0;
        }
        else
        {
            if (arrvTime < startTime + k)
            {
                maxRate = std::max(maxRate, rate);
            }
            else
            {
                fairShareRate = maxRate;
                startTime = arrvTime;

                // UPDATE: In the original CSFQ code, when alpha is set to 0, packet dropping is skipped!
                if (currentQueueSize < queueThreshold)
                {
                    fairShareRate = 0.0;
                }
                else
                {
                    maxRate = 0.0;
                }
            }
        }
    }
}

