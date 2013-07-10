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
    numBitsReceived = 0;
    numPktsConformed = 0;
    numPktsReceived = 0;
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

            if (simTime() >= simulation.getWarmupPeriod()) {
                numBitsConformed += pktLength;
                numPktsConformed++;
            }

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
    std::stringstream ss_received, ss_conformed, ss_throughput;
    ss_received << "number of packets received";
    ss_conformed << "number of packets conformed";
    ss_throughput << "bits/sec received";
    recordScalar((ss_received.str()).c_str(), numPktsReceived);
    recordScalar((ss_conformed.str()).c_str(), numPktsConformed);
    recordScalar((ss_throughput.str()).c_str(), numBitsReceived / (simTime() - simulation.getWarmupPeriod()).dbl());
    recordScalar("packet conformed rate", numPktsConformed / double(numPktsReceived));
}
