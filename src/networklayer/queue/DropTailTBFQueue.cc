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

    // configuration
    burstSize = par("burstSize").longValue()*8; // in bit
    meanRate = par("meanRate"); // in bps
    mtu = par("mtu").longValue()*8; // in bit
    peakRate = par("peakRate"); // in bps

    // state
    meanBucketLength = burstSize;
    peakBucketLength = mtu;
    lastTime = simTime();
    isTxScheduled = false;
// DEBUG
    numQueueSent = 0;
// DEBUG

    // timers
    resumeTransmissionTimer = new cMessage("Resume frame transmission");
}

void DropTailTBFQueue::handleMessage(cMessage *msg)
{
    if (msg == resumeTransmissionTimer)
    {
        // resume frame transmission (scheduled from the previous shaping)
        isTxScheduled = false;
        cMessage *queuedMsg = (cMessage *) queue.front();
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
//    else if (msg->arrivedOn("in"))
    else if (dynamic_cast<EtherFrame *>(msg) != NULL)
    {
        numQueueReceived++;
        if ((packetRequested > 0) && (!isTxScheduled))
        {
            if (isConformed(msg))
            {
                packetRequested--;
// DEBUG
                numQueueSent++;
// DEBUG
                sendOut(msg);
            }
            else
            {
                numQueueShaped++;
                int pktLength = (check_and_cast<cPacket *>(msg))->getBitLength();
                bool dropped = enqueue(msg);
                if (dropped) {
                    numQueueDropped++;
                }
                else {
                    // schedule frame transmission when enough tokens will be available
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
// DEBUG
    EV << "DropTailTBFQueue::requestPacket() is called" << endl;
// DEBUG

    cMessage *msg = (cMessage *) queue.front();
    if ((msg == NULL) || isTxScheduled)
    {
        packetRequested++;
    }
    else
    {
        if (isConformed(msg))
        {
            cMessage *msg = dequeue();
// DEBUG
            numQueueSent++;
// DEBUG
            sendOut(msg);
        }
        else
        {
            numQueueShaped++;
            // schedule frame transmission when enough tokens will be available
            scheduleTransmission((check_and_cast<cPacket *>(msg))->getBitLength());
            isTxScheduled = true;
        }
    }
}

bool DropTailTBFQueue::isConformed(cMessage *msg)
{
// DEBUG
    EV << "DropTailTBFQueue::isConformed() is called" << endl;
    EV << "Last Time = " << lastTime << endl;
    EV << "Current Time = " << simTime() << endl;
// DEBUG

    // update states
    simtime_t now = simTime();
    // meanBucketLength += int(meanRate*(now - lastTime).dbl() + 0.5);
    unsigned long long meanTemp = meanBucketLength + (unsigned long long)(meanRate*(now - lastTime).dbl() + 0.5);
    meanBucketLength = int((meanTemp > burstSize) ? burstSize : meanTemp);
    // peakBucketLength += int(peakRate*(now - lastTime).dbl() + 0.5);
    unsigned long long peakTemp = peakBucketLength + (unsigned long long)(peakRate*(now - lastTime).dbl() + 0.5);
    peakBucketLength = int((peakTemp > mtu) ? mtu : peakTemp);
    lastTime = now;

// DEBUG
    if (msg == NULL)
    {
        error("msg is NULL inside isConformed()");
    }
// DEBUG

    int pktLength = (check_and_cast<cPacket *>(msg))->getBitLength();
// DEBUG
    EV << "Packet Length = " << pktLength << endl;
// DEBUG
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

void DropTailTBFQueue::scheduleTransmission(int pktLength)
{
    double meanDelay = (pktLength - meanBucketLength) / meanRate;
    double peakDelay = (pktLength - peakBucketLength) / peakRate;

// DEBUG
    EV << "Packet Length = " << pktLength << endl;
    EV << "DropTailTBFQueue::scheduleTransmission() is called" << endl;
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
    EV << "Token bucket for mean rate/burst control " << endl;
    EV << "- Burst size [bit]: " << burstSize << endl;
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
// DEBUG
    recordScalar("packets sent by queue", numQueueSent);
// DEBUG
}
