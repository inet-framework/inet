//
// Copyright (C) 2011 Kyeong Soo (Joseph) Kim
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


#include <omnetpp.h>
#include "DropTailTBFQueue.h"
#include "EtherMAC.h"

Define_Module(DropTailTBFQueue);

DropTailTBFQueue::DropTailTBFQueue()
{
    resumeTransmissionTimer = NULL;
}

DropTailTBFQueue::~DropTailTBFQueue()
{
    cancelAndDelete(resumeTransmissionTimer);
}

void DropTailTBFQueue::initialize()
{
    DropTailQueue::initialize();
    queue.setName("l2queue");

    // statistic
    numQueueShaped = 0;
    qlenVec.setName("queue length");
    dropVec.setName("drops");

    outGate = gate("out");

    // configuration
    frameCapacity = par("frameCapacity");
    burstSize = par("burstSize"); // in bytes
    meanRate = par("meanRate"); // in bps
    mtu = par("mtu"); // in bytes
    peakRate = par("peakRate"); // in bps

    // variables
    meanBucketLength = burstSize * 8;
    peakBucketLength = mtu * 8;
    lastTime = simTime();

    // timers
    resumeTransmissionTimer = new cMessage("Resume frame transmission");
}

void DropTailTBFQueue::handleMessage(cMessage *msg)
{
    if (msg == resumeTransmissionTimer)
    {
        // resume frame transmission (scheduled from the previous shaping)
        cMessage *queuedMsg = (cMessage *) queue.front();
        if (isConformed(queuedMsg))
        {
            requestPacket();
        }
        else
        {
            dumpTbfStatus();
            error("Resumed frame transmission (length=%d [bit]) is not conformed by TBF",
                    (check_and_cast<cPacket *>(queuedMsg))->getBitLength());
        }
    }
//    else if (msg->arrivedOn("in"))
    else if (dynamic_cast<EtherFrame *>(msg) != NULL)
    {
        numQueueReceived++;
        if (packetRequested > 0)
        {
            if (isConformed(msg))
            {
                packetRequested--;
                sendOut(msg);
            }
            else
            {
                // schedule frame transmission when enough tokens will be available
                scheduleTransmission(msg);
                bool dropped = enqueue(msg);
                if (dropped) {
                    numQueueDropped++;
                    cancelEvent(resumeTransmissionTimer);
                }
            }
        }
        else
        {
            bool dropped = enqueue(msg);
            if (dropped)
                numQueueDropped++;
        }

        if (ev.isGUI())
        {
            char buf[40];
            sprintf(buf, "q rcvd: %d\nq dropped: %d", numQueueReceived, numQueueDropped);
            getDisplayString().setTagArg("t", 0, buf);
        }
    }
    else
    {
        error("Unkown message received");
    }
}

void DropTailTBFQueue::requestPacket()
{
    Enter_Method("requestPacket()");

    cMessage *msg = (cMessage *) queue.front();
    if (msg == NULL)
    {
        packetRequested++;
    }
    else
    {
        if (isConformed(msg))
        {
            cMessage *msg = dequeue();
            sendOut(msg);
        }
        else
        {
            // schedule frame transmission when enough tokens will be available
            scheduleTransmission(msg);
        }
    }
}

bool DropTailTBFQueue::isConformed(cMessage *msg)
{
    // update TBF
    simtime_t now;
    meanBucketLength += int(meanRate*(now - lastTime).dbl() + 0.5);
    meanBucketLength = meanBucketLength > burstSize ? burstSize : meanBucketLength;
    peakBucketLength += int(peakRate*(now - lastTime).dbl() + 0.5);
    peakBucketLength += peakBucketLength > mtu ? mtu : peakBucketLength;
    lastTime = now;

// DEBUG
    if (msg == NULL)
    {
        error("msg is NULL inside isConformed()");
    }
// DEBUG

    int pktLength = (check_and_cast<cPacket *>(msg))->getBitLength();
    if (pktLength <= meanBucketLength)
    {
        if  (pktLength <= peakBucketLength)
        {
            meanBucketLength -= pktLength;
            peakBucketLength -= pktLength;
            return true;
        }
    }
    return false;
}

void DropTailTBFQueue::scheduleTransmission(cMessage *msg)
{
// DEBUG
    if (msg == NULL)
    {
        error("msg is NULL inside scheduleTransmission()");
    }
// DEBUG

    int pktLength = (check_and_cast<cPacket *>(msg))->getBitLength();
    double meanDelay = (pktLength - meanBucketLength) / meanRate;
    double peakDelay = (pktLength - peakBucketLength) / peakRate;

    scheduleAt(simTime() + max(meanDelay, peakDelay), resumeTransmissionTimer);
}

void DropTailTBFQueue::dumpTbfStatus()
{
    EV << "Token bucket for mean rate/burst control " << endl;
    EV << "- Burst size: " << burstSize << endl;
    EV << "- Mean rate: " << meanRate << endl;
    EV << "- Bucket length: " << meanBucketLength << endl;
    EV << "Token bucket for peak rate/MTU control " << endl;
    EV << "- MTU: " << mtu << endl;
    EV << "- Peak rate: " << peakRate << endl;
    EV << "- Bucket length: " << peakBucketLength << endl;
}

void DropTailTBFQueue::finish()
{
    DropTailQueue::finish();
    recordScalar("packets shaped by queue", numQueueShaped);
}
