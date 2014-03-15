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
    // NED parameters
    bucketSize = par("bucketSize").longValue()*8LL; // in bit
    meanRate = par("meanRate"); // in bps
    mtu = par("mtu").longValue()*8; // in bit
    peakRate = par("peakRate"); // in bps

    // TBF states
    meanBucketLength = bucketSize;
    peakBucketLength = mtu;
    lastTime = simTime();

    // statistics
    warmupFinished = false;
    numBitsConformed = 0L;
    numBitsMetered = 0L;
    numPktsConformed = 0;
    numPktsMetered = 0;
}

int BasicTokenBucketMeter::meterPacket(cMessage *msg) {
    Enter_Method
    ("meterPacket()");

    if (warmupFinished == false) {
        if (simTime() >= simulation.getWarmupPeriod()) {
            warmupFinished = true;
        }
    }

    int pktLength = (check_and_cast<cPacket *>(msg))->getBitLength();
    simtime_t now = simTime();

    if (warmupFinished == true) {
        numBitsMetered += pktLength;
        numPktsMetered++;
    }

//    // DEBUG
//    EV << getFullPath() << ":" << endl;
//    EV << "- Last Time = " << lastTime << endl;
//    EV << "- Current Time = " << now << endl;
//    EV << "- Packet Length = " << pktLength << endl;
//    double tokenGenerated = (unsigned long long) ceil(meanRate * (now - lastTime).dbl());
//    EV << "- Token Generated = " << tokenGenerated << endl;
//    EV << "- Mean Bucket Size [bit]: " << bucketSize << endl;
//    EV << "- Mean Rate [bps]: " << meanRate << endl;
//    EV << "- Mean Bucket Length [bit]: " << meanBucketLength << endl;
//    EV << "- Peak Bucket Size (MTU) [bit]: " << mtu << endl;
//    EV << "- Peak Rate [bps]: " << peakRate << endl;
//    EV << "- Peak Bucket length [bit]: " << peakBucketLength << endl;
//    // DEBUG

    // update states
    //unsigned long long meanTemp = meanBucketLength + (unsigned long long)(meanRate*(now - lastTime).dbl() + 0.5);
    unsigned long long meanTemp = meanBucketLength
            + (unsigned long long)ceil(meanRate * (now - lastTime).dbl());
    meanBucketLength = (unsigned long long) ((meanTemp > bucketSize) ? bucketSize : meanTemp);
    //unsigned long long peakTemp = peakBucketLength + (unsigned long long)(peakRate*(now - lastTime).dbl() + 0.5);
    unsigned long long peakTemp = peakBucketLength
            + (unsigned long long)ceil(peakRate * (now - lastTime).dbl());
    peakBucketLength = int((peakTemp > (unsigned long long)mtu) ? mtu : peakTemp);
    lastTime = now;

    if ((unsigned long long)pktLength <= meanBucketLength) {
        if (pktLength <= peakBucketLength) {
            meanBucketLength -= pktLength;
            peakBucketLength -= pktLength;

            if (warmupFinished == true) {
                numBitsConformed += pktLength;
                numPktsConformed++;
            }

            return 0;   // conformed
        }
    }
    return 1;   // not conformed
}

void BasicTokenBucketMeter::dumpStatus() {
    simtime_t now = simTime();
    double tokenGenerated = (unsigned long long) ceil(meanRate * (now - lastTime).dbl());

    EV << getFullPath() << ":" << endl;
    EV << getFullPath() << ":" << endl;
    EV << "- Last Time = " << lastTime << endl;
    EV << "- Current Time = " << now << endl;
    EV << "- Token Generated = " << tokenGenerated << endl;
    EV << "- Mean Bucket Size [bit]: " << bucketSize << endl;
    EV << "- Mean Rate [bps]: " << meanRate << endl;
    EV << "- Mean Bucket Length [bit]: " << meanBucketLength << endl;
    EV << "- Peak Bucket Size (MTU) [bit]: " << mtu << endl;
    EV << "- Peak Rate [bps]: " << peakRate << endl;
    EV << "- Peak Bucket length [bit]: " << peakBucketLength << endl;
}

void BasicTokenBucketMeter::finish()
{
    std::stringstream ss_metered, ss_conformed, ss_throughput;
    ss_metered << "number of packets metered";
    ss_conformed << "number of packets conformed";
    ss_throughput << "bits/sec metered";
    recordScalar((ss_metered.str()).c_str(), numPktsMetered);
    recordScalar((ss_conformed.str()).c_str(), numPktsConformed);
    recordScalar((ss_throughput.str()).c_str(), numBitsMetered / (simTime() - simulation.getWarmupPeriod()).dbl());
    recordScalar("packet conformed rate", numPktsConformed / double(numPktsMetered));
}
