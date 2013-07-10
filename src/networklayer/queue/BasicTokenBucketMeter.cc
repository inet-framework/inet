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


#include "BasicTokenBucketMeter.h"

Define_Module(BasicTokenBucketMeter);

void BasicTokenBucketMeter::initialize()
{
    // configuration
    bucketSize = par("bucketSize").longValue()*8LL; // in bit
    meanRate = par("meanRate"); // in bps
    mtu = par("mtu").longValue()*8; // in bit
    peakRate = par("peakRate"); // in bps

    // statistic
    warmupFinished = false;
    numBitsConformed = 0;
    numBitsSent = 0;
    numPktsConformed = 0;
    numPktsSent = 0;
}

int BasicTokenBucketMeter::meterPacket(cMessage *msg)
{
    Enter_Method("isConformed()");

    int pktLength = (check_and_cast<cPacket *>(msg))->getBitLength();

// DEBUG
    EV << "Last Time = " << lastTime << endl;
    EV << "Current Time = " << simTime() << endl;
    EV << "Packet Length = " << pktLength << endl;
// DEBUG

    // update states
    simtime_t now = simTime();
    //unsigned long long meanTemp = meanBucketLength + (unsigned long long)(meanRate*(now - lastTime).dbl() + 0.5);
    unsigned long long meanTemp = meanBucketLength + (unsigned long long)ceil(meanRate*(now - lastTime).dbl());
    meanBucketLength = (long long)((meanTemp > bucketSize) ? bucketSize : meanTemp);
    //unsigned long long peakTemp = peakBucketLength + (unsigned long long)(peakRate*(now - lastTime).dbl() + 0.5);
    unsigned long long peakTemp = peakBucketLength + (unsigned long long)ceil(peakRate*(now - lastTime).dbl());
    peakBucketLength = int((peakTemp > mtu) ? mtu : peakTemp);
    lastTime = now;

    if (pktLength <= meanBucketLength)
    {
        if  (pktLength <= peakBucketLength)
        {
            meanBucketLength -= pktLength;
            peakBucketLength -= pktLength;
            return 1;   // conformed
        }
    }
    return 0;   // not conformed
}

//void BasicTokenBucketMeter::dumpTbfStatus(int queueIndex)
//{
//    EV << "Last Time = " << lastTime[queueIndex] << endl;
//    EV << "Current Time = " << simTime() << endl;
//    EV << "Token bucket for mean rate/burst control " << endl;
//    EV << "- Bucket size [bit]: " << bucketSize << endl;
//    EV << "- Mean rate [bps]: " << meanRate << endl;
//    EV << "- Bucket length [bit]: " << meanBucketLength[queueIndex] << endl;
//    EV << "Token bucket for peak rate/MTU control " << endl;
//    EV << "- MTU [bit]: " << mtu << endl;
//    EV << "- Peak rate [bps]: " << peakRate << endl;
//    EV << "- Bucket length [bit]: " << peakBucketLength[queueIndex] << endl;
//}

void BasicTokenBucketMeter::finish()
{
    unsigned long sumQueueReceived = 0;
    unsigned long sumQueueDropped = 0;
    unsigned long sumQueueShaped = 0;
    unsigned long sumQueueUnshaped = 0;

    for (int i=0; i < numQueues; i++)
    {
        std::stringstream ss_received, ss_dropped, ss_shaped, ss_sent, ss_throughput;
        ss_received << "packets received by per-VLAN queue[" << i << "]";
        ss_dropped << "packets dropped by per-VLAN queue[" << i << "]";
        ss_shaped << "packets shaped by per-VLAN queue[" << i << "]";
        ss_sent << "packets sent by per-VLAN queue[" << i << "]";
        ss_throughput << "bits/sec sent per-VLAN queue[" << i << "]";
        recordScalar((ss_received.str()).c_str(), numQueueReceived[i]);
        recordScalar((ss_dropped.str()).c_str(), numQueueDropped[i]);
        recordScalar((ss_shaped.str()).c_str(), numQueueReceived[i]-numQueueUnshaped[i]);
        recordScalar((ss_sent.str()).c_str(), numQueueSent[i]);
        recordScalar((ss_throughput.str()).c_str(), numBitsSent[i]/(simTime()-simulation.getWarmupPeriod()).dbl());
        sumQueueReceived += numQueueReceived[i];
        sumQueueDropped += numQueueDropped[i];
        sumQueueShaped += numQueueReceived[i] - numQueueUnshaped[i];
    }
    recordScalar("overall packet loss rate of per-VLAN queues", sumQueueDropped/double(sumQueueReceived));
    recordScalar("overall packet shaped rate of per-VLAN queues", sumQueueShaped/double(sumQueueReceived));
}
