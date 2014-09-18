/***************************************************************************
                          RTPReceiverInfo.cc  -  description
                             -------------------
    (C) 2007 Ahmed Ayadi  <ahmed.ayadi@sophia.inria.fr>
    (C) 2001 Matthias Oppitz <Matthias.Oppitz@gmx.de>

***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "inet/transportlayer/rtp/RTPReceiverInfo.h"

#include "inet/transportlayer/rtp/reports.h"
#include "inet/transportlayer/rtp/RTPPacket.h"

namespace inet {

namespace rtp {

Register_Class(RTPReceiverInfo);

RTPReceiverInfo::RTPReceiverInfo(uint32 ssrc) : RTPParticipantInfo(ssrc)
{
    _sequenceNumberBase = 0;
    _highestSequenceNumber = 0;
    _highestSequenceNumberPrior = 0;
    _sequenceNumberCycles = 0;

    _packetsReceived = 0;
    _packetsReceivedPrior = 0;

    _jitter = 0.0;
    _clockRate = 0;
    _lastSenderReportRTPTimeStamp = 0;
    _lastSenderReportNTPTimeStamp = 0;
    _lastPacketRTPTimeStamp = 0;

    _lastPacketArrivalTime = 0.0;
    _lastSenderReportArrivalTime = 0.0;

    _inactiveIntervals = 0;
    _startOfInactivity = 0.0;
    _itemsReceived = 0;
    packetSequenceLostLogFile = NULL;
}

RTPReceiverInfo::RTPReceiverInfo(const RTPReceiverInfo& receiverInfo) : RTPParticipantInfo(receiverInfo)
{
    copy(receiverInfo);
}

RTPReceiverInfo::~RTPReceiverInfo()
{
}

RTPReceiverInfo& RTPReceiverInfo::operator=(const RTPReceiverInfo& receiverInfo)
{
    if (this == &receiverInfo)
        return *this;
    RTPParticipantInfo::operator=(receiverInfo);
    copy(receiverInfo);
    return *this;
}

void RTPReceiverInfo::copy(const RTPReceiverInfo& receiverInfo)
{
    _sequenceNumberBase = receiverInfo._sequenceNumberBase;
    _highestSequenceNumber = receiverInfo._highestSequenceNumber;
    _highestSequenceNumberPrior = receiverInfo._highestSequenceNumberPrior;
    _sequenceNumberCycles = receiverInfo._sequenceNumberCycles;

    _packetsReceived = receiverInfo._packetsReceived;
    _packetsReceivedPrior = receiverInfo._packetsReceivedPrior;

    _jitter = receiverInfo._jitter;
    _clockRate = receiverInfo._clockRate;
    _lastSenderReportRTPTimeStamp = receiverInfo._lastSenderReportRTPTimeStamp;
    _lastSenderReportNTPTimeStamp = receiverInfo._lastSenderReportNTPTimeStamp;
    _lastPacketRTPTimeStamp = receiverInfo._lastPacketRTPTimeStamp;

    _lastPacketArrivalTime = receiverInfo._lastPacketArrivalTime;
    _lastSenderReportArrivalTime = receiverInfo._lastSenderReportArrivalTime;

    _inactiveIntervals = receiverInfo._inactiveIntervals;
    _startOfInactivity = receiverInfo._startOfInactivity;
    _itemsReceived = receiverInfo._itemsReceived;
}

RTPReceiverInfo *RTPReceiverInfo::dup() const
{
    return new RTPReceiverInfo(*this);
}

void RTPReceiverInfo::processRTPPacket(RTPPacket *packet, int id, simtime_t arrivalTime)
{
    // this endsystem sends, it isn't inactive
    _inactiveIntervals = 0;

    _packetsReceived++;
    _itemsReceived++;

    if (_packetsReceived == 1) {
        _sequenceNumberBase = packet->getSequenceNumber();
    }
    else {
        /*if (packet->getSequenceNumber() > _highestSequenceNumber+1)
           {
            _packetLostOutVector.record(packet->getSequenceNumber() - _highestSequenceNumber -1);
            for (int i = _highestSequenceNumber+1; i< packet->getSequenceNumber(); i++ )
            {
                //std::cout << "id = "<< id <<" SequeceNumber loss = "<<i<<endl;
                packetSequenceLostLogFile = fopen ("PacketLossLog.log","+w");
                if (packetSequenceLostLogFile != NULL)
                {
                    //sprintf (line, "id = %d SequeceNumber loss = %f ", id,i);
                    fputs (i, packetSequenceLostLogFile);
                    fclose (packetSequenceLostLogFile);
                }
            }
           }*/

        if (packet->getSequenceNumber() > _highestSequenceNumber) {
            // it is possible that this is a late packet from the
            // previous sequence wrap
            if (!(packet->getSequenceNumber() > 0xFFEF && _highestSequenceNumber < 0x10))
                _highestSequenceNumber = packet->getSequenceNumber();
        }
        else {
            // is it a sequence number wrap around 0xFFFF to 0x0000 ?
            if (packet->getSequenceNumber() < 0x10 && _highestSequenceNumber > 0xFFEF) {
                _sequenceNumberCycles += 0x00010000;
                _highestSequenceNumber = packet->getSequenceNumber();
            }
        }
        // calculate interarrival jitter
        if (_clockRate != 0) {
            simtime_t d = packet->getTimeStamp() - _lastPacketRTPTimeStamp
                - (arrivalTime - _lastPacketArrivalTime) * (double)_clockRate;
            if (d < 0)
                d = -d;
            _jitter = _jitter + (d - _jitter) / 16;
        }

        _lastPacketRTPTimeStamp = packet->getTimeStamp();
        _lastPacketArrivalTime = arrivalTime;
    }

    RTPParticipantInfo::processRTPPacket(packet, id, arrivalTime);
}

void RTPReceiverInfo::processSenderReport(SenderReport *report, simtime_t arrivalTime)
{
    _lastSenderReportArrivalTime = arrivalTime;
    if (_lastSenderReportRTPTimeStamp == 0) {
        _lastSenderReportRTPTimeStamp = report->getRTPTimeStamp();
        _lastSenderReportNTPTimeStamp = report->getNTPTimeStamp();
    }
    else if (_clockRate == 0) {
        uint32 rtpTicks = report->getRTPTimeStamp() - _lastSenderReportRTPTimeStamp;
        uint64 ntpDifference = report->getNTPTimeStamp() - _lastSenderReportNTPTimeStamp;
        long double ntpSeconds = (long double)ntpDifference / (long double)(0xFFFFFFFF);
        _clockRate = (int)((long double)rtpTicks / ntpSeconds);
    }

    _inactiveIntervals = 0;
    _itemsReceived++;

    delete report;
}

void RTPReceiverInfo::processSDESChunk(SDESChunk *sdesChunk, simtime_t arrivalTime)
{
    RTPParticipantInfo::processSDESChunk(sdesChunk, arrivalTime);
    _itemsReceived++;
    _inactiveIntervals = 0;
}

ReceptionReport *RTPReceiverInfo::receptionReport(simtime_t now)
{
    if (isSender()) {
        ReceptionReport *receptionReport = new ReceptionReport();
        receptionReport->setSsrc(getSsrc());

        uint64 packetsExpected = _sequenceNumberCycles + (uint64)_highestSequenceNumber
            - (uint64)_sequenceNumberBase + (uint64)1;
        uint64 packetsLost = packetsExpected - _packetsReceived;

        int32 packetsExpectedInInterval =
            _sequenceNumberCycles + _highestSequenceNumber - _highestSequenceNumberPrior;
        int32 packetsReceivedInInterval = _packetsReceived - _packetsReceivedPrior;
        int32 packetsLostInInterval = packetsExpectedInInterval - packetsReceivedInInterval;
        uint8 fractionLost = 0;
        if (packetsLostInInterval > 0) {
            fractionLost = (packetsLostInInterval << 8) / packetsExpectedInInterval;
        }

        receptionReport->setFractionLost(fractionLost);
        receptionReport->setPacketsLostCumulative(packetsLost);
        receptionReport->setSequenceNumber(_sequenceNumberCycles + _highestSequenceNumber);

        receptionReport->setJitter((uint32)SIMTIME_DBL(_jitter));    //XXX ??? store it in secs? --Andras

        // the middle 32 bit of the ntp time stamp of the last sender report
        receptionReport->setLastSR((_lastSenderReportNTPTimeStamp >> 16) & 0xFFFFFFFF);

        // the delay since the arrival of the last sender report in units
        // of 1 / 65536 seconds
        // 0 if no sender report has ben received
        receptionReport->setDelaySinceLastSR(_lastSenderReportArrivalTime == 0.0 ? 0
                : (uint32)(SIMTIME_DBL(now - _lastSenderReportArrivalTime) * 65536.0));

        return receptionReport;
    }
    else
        return NULL;
}

void RTPReceiverInfo::nextInterval(simtime_t now)
{
    _inactiveIntervals++;
    if (_inactiveIntervals == MAX_INACTIVE_INTERVALS) {
        _startOfInactivity = now;
    }
    _highestSequenceNumberPrior = _highestSequenceNumber + _sequenceNumberCycles;
    _packetsReceivedPrior = _packetsReceived;
    RTPParticipantInfo::nextInterval(now);
}

bool RTPReceiverInfo::isActive()
{
    return _inactiveIntervals < MAX_INACTIVE_INTERVALS;
}

bool RTPReceiverInfo::isValid()
{
    return _itemsReceived >= MAX_INACTIVE_INTERVALS;
}

bool RTPReceiverInfo::toBeDeleted(simtime_t now)
{
    // an RTP system should be removed from the list of known systems
    // when it hasn't been validated and hasn't been active for
    // 5 rtcp intervals or if it has been validated and has been
    // inactive for 30 minutes
    return (!isValid() && !isActive()) || (isValid() && !isActive() && (now - _startOfInactivity > 60.0 * 30.0));
}

} // namespace rtp

} // namespace inet

