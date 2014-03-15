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

    // statistic
    numQueueShaped = 0;
    numQueueSent = 0;

    // configuration
    bucketSize = par("bucketSize").longValue()*8LL; // in bit
    meanRate = par("meanRate"); // in bps
    mtu = par("mtu").longValue()*8; // in bit
    peakRate = par("peakRate"); // in bps

    // state
    meanBucketLength = bucketSize;
    peakBucketLength = mtu;
    lastTime = simTime();
    isTxScheduled = false;

    // timers
    resumeTransmissionTimer = new cMessage("Resume frame transmission");
}

void DropTailTBFQueue::handleMessage(cMessage *msg)
{
    if (msg == resumeTransmissionTimer)
    {
        isTxScheduled = false;
        requestPacket();
        // if (isConformed(queuedMsg))
        // {
        //     requestPacket();
        // }
        // else
        // {
        //     dumpTbfStatus();
        //     error("Resumed frame transmission (length=%d [bit]) is not conformed by TBF",
        //             (check_and_cast<cPacket *>(queuedMsg))->getBitLength());
        // }
    }
    // else if (msg->arrivedOn("in"))
    // else if (dynamic_cast<EtherFrame *>(msg) != NULL)
    else
    {
        numPktsReceived++;
        int pktLength = (check_and_cast<cPacket *>(msg))->getBitLength();
// DEBUG
        ASSERT(pktLength > 0);
// DEBUG
        if ((packetRequested > 0) && (!isTxScheduled))
        {
            if (isConformed(pktLength))
            {
                packetRequested--;
                numQueueSent++;
                sendOut(msg);
            }
            else
            {
                numQueueShaped++;
                bool dropped = enqueue(msg);
                if (dropped) {
                    numQueueDropped++;
                }
                else {
                    scheduleTransmission(pktLength);
                    isTxScheduled = true;
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
            sprintf(buf, "q rcvd: %d\nq dropped: %d", numPktsReceived, numQueueDropped);
            getDisplayString().setTagArg("t", 0, buf);
        }
    }
}

void DropTailTBFQueue::requestPacket()
{
    Enter_Method("requestPacket()");

    cMessage *msg = (cMessage *) queue.front();
    if ((msg == NULL) || isTxScheduled)
    {
        packetRequested++;
    }
    else
    {
        int pktLength = (check_and_cast<cPacket *>(msg))->getBitLength();
        if (isConformed(pktLength))
        {
            cMessage *msg = dequeue();
            numQueueSent++;
            sendOut(msg);
        }
        else
        {
            numQueueShaped++;
            scheduleTransmission(pktLength);
            isTxScheduled = true;
        }
    }
}

bool DropTailTBFQueue::isConformed(int pktLength)
{
// DEBUG
    EV << "DropTailTBFQueue::isConformed() is called" << endl;
    EV << "Last Time = " << lastTime << endl;
    EV << "Current Time = " << simTime() << endl;
    EV << "Packet Length = " << pktLength << endl;
// DEBUG

    // update states
    simtime_t now = simTime();
    unsigned long long meanTemp = meanBucketLength + (unsigned long long)(meanRate*(now - lastTime).dbl() + 0.5);
    // unsigned long long meanTemp = meanBucketLength + (unsigned long long)(meanRate*(now - lastTime).dbl());
    meanBucketLength = (long long)((meanTemp > bucketSize) ? bucketSize : meanTemp);
    unsigned long long peakTemp = peakBucketLength + (unsigned long long)(peakRate*(now - lastTime).dbl() + 0.5);
    // unsigned long long peakTemp = peakBucketLength + (unsigned long long)(peakRate*(now - lastTime).dbl());
    peakBucketLength = int((peakTemp > mtu) ? mtu : peakTemp);
    lastTime = now;

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

// schedule frame transmission when enough tokens will be available
void DropTailTBFQueue::scheduleTransmission(int pktLength)
{
    double meanDelay = (pktLength - meanBucketLength) / meanRate;
    double peakDelay = (pktLength - peakBucketLength) / peakRate;

// DEBUG
    EV << "DropTailTBFQueue::scheduleTransmission() is called" << endl;
    EV << "Packet Length = " << pktLength << endl;
    dumpTbfStatus();
    EV << "Delay for Mean TBF = " << meanDelay << endl;
    EV << "Delay for Peak TBF = " << peakDelay << endl;
    EV << "Current Time = " << simTime() << endl;
    EV << "Scheduled Time = " << simTime() + max(meanDelay, peakDelay) << endl;
// DEBUG

    scheduleAt(simTime() + max(meanDelay, peakDelay), resumeTransmissionTimer);
}

void DropTailTBFQueue::dumpTbfStatus()
{
    EV << "Last Time = " << lastTime << endl;
    EV << "Current Time = " << simTime() << endl;
    EV << "Token bucket for mean rate/burst control " << endl;
    EV << "- Bucket size [bit]: " << bucketSize << endl;
    EV << "- Mean rate [bps]: " << meanRate << endl;
    EV << "- Bucket length [bit]: " << meanBucketLength << endl;
    EV << "Token bucket for peak rate/MTU control " << endl;
    EV << "- MTU [bit]: " << mtu << endl;
    EV << "- Peak rate [bps]: " << peakRate << endl;
    EV << "- Bucket length [bit]: " << peakBucketLength << endl;
}

void DropTailTBFQueue::finish()
{
    DropTailQueue::finish();
    recordScalar("packets shaped by queue", numQueueShaped);
    recordScalar("packets sent by queue", numQueueSent);

// DEBUG
    dumpTbfStatus();
    EV << "Current Queue Length = " << queue.length() << endl;
// DEBUG
}
