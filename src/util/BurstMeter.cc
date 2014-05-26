///
/// @file   BurstMeter.cc
/// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
/// @date   Mar/1/2012
///
/// @brief  Implements 'BurstMeter' class for measuring frame/packet bursts.
///
/// @remarks Copyright (C) 2012 Kyeong Soo (Joseph) Kim. All rights reserved.
///
/// @remarks This software is written and distributed under the GNU General
///          Public License Version 2 (http://www.gnu.org/licenses/gpl-2.0.html).
///          You must not remove this notice, or any other, from this software.
///


#include "BurstMeter.h"

// Register modules.
Define_Module(BurstMeter);

void BurstMeter::initialize()
{
    maxIFG = par("maxIFG").longValue();

    // set "flow-through" reception mode for all input gates
    gate("in")->setDeliverOnReceptionStart(true);

    // initialize maximum frame gap to detect bursts
    cChannel *cIn = gate("in")->getIncomingTransmissionChannel();
    maxFrameGap = maxIFG * INTERFRAME_GAP_BITS / cIn->getNominalDatarate();

    receptionTime = 0.0;
    macAddress = "ffffffffffff";

    packetBurstSignal = registerSignal("packetBurst");
    burst = 0;
    numBursts = 0;
    sumBursts = 0;
    WATCH(numBursts);
}

void BurstMeter::handleMessage(cMessage *msg)
{
    if (simTime() >= simulation.getWarmupPeriod())
    {
        cPacket *pkt = (cPacket *) msg;
        simtime_t startTime = pkt->getArrivalTime();
        simtime_t endTime = startTime + pkt->getDuration();

        if (dynamic_cast<EtherFrame *>(msg) != NULL)
        {
            MACAddress newAddress = ((EtherFrame *) msg)->getDest();
            if ((newAddress == macAddress) && ((startTime - receptionTime) <= maxFrameGap))
            { // this is the continuation of the existing burst
                burst++;
            }
            else
            { // detect a new burst
                if (burst > 0)
                {   // emit signal and update statistics for the existing burst
                    emit(packetBurstSignal, burst);
                    numBursts++;
                    sumBursts += burst;
                }
                // initialize for the new burst
                burst = 1;
                macAddress = newAddress;
            }
        } // end of if() for Ethernet frames

        // TODO: Implement processing for other frame/packet formats

        receptionTime = endTime;
    } // end of if() for warmup processing

    send(msg, "out");
}

void BurstMeter::finish()
{
    // emit signal and update statistics for the current burst first
    if (burst > 0)
    {
        emit(packetBurstSignal, burst);
        numBursts++;
        sumBursts += burst;
    }

    // record session statistics
    if (numBursts > 0)
    {
        double avgBurst = sumBursts/double(numBursts);
        recordScalar("number of bursts measured", numBursts);
        recordScalar("average packet bursts", avgBurst);
    }
}
