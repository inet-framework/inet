//
// Copyright (C) 2001 Matthias Oppitz <Matthias.Oppitz@gmx.de>
// Copyright (C) 2007 Ahmed Ayadi <ahmed.ayadi@sophia.inria.fr>
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/transportlayer/rtp/RtpReceiverInfo.h"

#include "inet/transportlayer/rtp/Reports_m.h"
#include "inet/transportlayer/rtp/RtpPacket_m.h"

namespace inet {
namespace rtp {

Register_Class(RtpReceiverInfo);

RtpReceiverInfo::RtpReceiverInfo(uint32_t ssrc) : RtpParticipantInfo(ssrc)
{
}

RtpReceiverInfo::RtpReceiverInfo(const RtpReceiverInfo& receiverInfo) : RtpParticipantInfo(receiverInfo)
{
    copy(receiverInfo);
}

RtpReceiverInfo::~RtpReceiverInfo()
{
}

RtpReceiverInfo& RtpReceiverInfo::operator=(const RtpReceiverInfo& receiverInfo)
{
    if (this == &receiverInfo)
        return *this;
    RtpParticipantInfo::operator=(receiverInfo);
    copy(receiverInfo);
    return *this;
}

void RtpReceiverInfo::copy(const RtpReceiverInfo& receiverInfo)
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

RtpReceiverInfo *RtpReceiverInfo::dup() const
{
    return new RtpReceiverInfo(*this);
}

void RtpReceiverInfo::processRTPPacket(Packet *packet, int id, simtime_t arrivalTime)
{
    const auto& rtpHeader = packet->peekAtFront<RtpHeader>();
    // this endsystem sends, it isn't inactive
    _inactiveIntervals = 0;

    _packetsReceived++;
    _itemsReceived++;

    if (_packetsReceived == 1) {
        _sequenceNumberBase = rtpHeader->getSequenceNumber();
    }
    else {
        /*if (packet->getSequenceNumber() > _highestSequenceNumber+1)
           {
            _packetLostOutVector.record(packet->getSequenceNumber() - _highestSequenceNumber -1);
            for (int i = _highestSequenceNumber+1; i< packet->getSequenceNumber(); i++ )
            {
//                std::cout << "id = "<< id <<" SequeceNumber loss = "<<i<<endl;
                packetSequenceLostLogFile = fopen ("PacketLossLog.log","+w");
                if (packetSequenceLostLogFile != nullptr)
                {
//                    sprintf (line, "id = %d SequeceNumber loss = %f ", id,i);
                    fputs (i, packetSequenceLostLogFile);
                    fclose (packetSequenceLostLogFile);
                }
            }
           }*/

        if (rtpHeader->getSequenceNumber() > _highestSequenceNumber) {
            // it is possible that this is a late packet from the
            // previous sequence wrap
            if (!(rtpHeader->getSequenceNumber() > 0xFFEF && _highestSequenceNumber < 0x10))
                _highestSequenceNumber = rtpHeader->getSequenceNumber();
        }
        else {
            // is it a sequence number wrap around 0xFFFF to 0x0000 ?
            if (rtpHeader->getSequenceNumber() < 0x10 && _highestSequenceNumber > 0xFFEF) {
                _sequenceNumberCycles += 0x00010000;
                _highestSequenceNumber = rtpHeader->getSequenceNumber();
            }
        }
        // calculate interarrival jitter
        if (_clockRate != 0) {
            simtime_t d = rtpHeader->getTimeStamp() - _lastPacketRTPTimeStamp
                - (arrivalTime - _lastPacketArrivalTime) * (double)_clockRate;
            if (d < 0)
                d = -d;
            _jitter = _jitter + (d - _jitter) / 16;
        }

        _lastPacketRTPTimeStamp = rtpHeader->getTimeStamp();
        _lastPacketArrivalTime = arrivalTime;
    }

    RtpParticipantInfo::processRTPPacket(packet, id, arrivalTime);
}

void RtpReceiverInfo::processSenderReport(SenderReport *report, simtime_t arrivalTime)
{
    _lastSenderReportArrivalTime = arrivalTime;
    if (_lastSenderReportRTPTimeStamp == 0) {
        _lastSenderReportRTPTimeStamp = report->getRTPTimeStamp();
        _lastSenderReportNTPTimeStamp = report->getNTPTimeStamp();
    }
    else if (_clockRate == 0) {
        uint32_t rtpTicks = report->getRTPTimeStamp() - _lastSenderReportRTPTimeStamp;
        uint64_t ntpDifference = report->getNTPTimeStamp() - _lastSenderReportNTPTimeStamp;
        long double ntpSeconds = (long double)ntpDifference / (long double)(0xFFFFFFFF);
        _clockRate = (int)((long double)rtpTicks / ntpSeconds);
    }

    _inactiveIntervals = 0;
    _itemsReceived++;

    delete report;
}

void RtpReceiverInfo::processSDESChunk(const SdesChunk *sdesChunk, simtime_t arrivalTime)
{
    RtpParticipantInfo::processSDESChunk(sdesChunk, arrivalTime);
    _itemsReceived++;
    _inactiveIntervals = 0;
}

ReceptionReport *RtpReceiverInfo::receptionReport(simtime_t now)
{
    if (isSender()) {
        ReceptionReport *receptionReport = new ReceptionReport();
        receptionReport->setSsrc(getSsrc());

        uint64_t packetsExpected = _sequenceNumberCycles + (uint64_t)_highestSequenceNumber
            - (uint64_t)_sequenceNumberBase + (uint64_t)1;
        uint64_t packetsLost = packetsExpected - _packetsReceived;

        int32_t packetsExpectedInInterval =
            _sequenceNumberCycles + _highestSequenceNumber - _highestSequenceNumberPrior;
        int32_t packetsReceivedInInterval = _packetsReceived - _packetsReceivedPrior;
        int32_t packetsLostInInterval = packetsExpectedInInterval - packetsReceivedInInterval;
        uint8_t fractionLost = 0;
        if (packetsLostInInterval > 0) {
            fractionLost = (packetsLostInInterval << 8) / packetsExpectedInInterval;
        }

        receptionReport->setFractionLost(fractionLost);
        receptionReport->setPacketsLostCumulative(packetsLost);
        receptionReport->setSequenceNumber(_sequenceNumberCycles + _highestSequenceNumber);

        receptionReport->setJitter((uint32_t)SIMTIME_DBL(_jitter)); // FIXME store it in secs? --Andras

        // the middle 32 bit of the ntp time stamp of the last sender report
        receptionReport->setLastSR((_lastSenderReportNTPTimeStamp >> 16) & 0xFFFFFFFF);

        // the delay since the arrival of the last sender report in units
        // of 1 / 65536 seconds
        // 0 if no sender report has ben received
        receptionReport->setDelaySinceLastSR(_lastSenderReportArrivalTime == 0.0 ? 0
                : (uint32_t)(SIMTIME_DBL(now - _lastSenderReportArrivalTime) * 65536.0));

        return receptionReport;
    }
    else
        return nullptr;
}

void RtpReceiverInfo::nextInterval(simtime_t now)
{
    _inactiveIntervals++;
    if (_inactiveIntervals == MAX_INACTIVE_INTERVALS) {
        _startOfInactivity = now;
    }
    _highestSequenceNumberPrior = _highestSequenceNumber + _sequenceNumberCycles;
    _packetsReceivedPrior = _packetsReceived;
    RtpParticipantInfo::nextInterval(now);
}

bool RtpReceiverInfo::isActive()
{
    return _inactiveIntervals < MAX_INACTIVE_INTERVALS;
}

bool RtpReceiverInfo::isValid()
{
    return _itemsReceived >= MAX_INACTIVE_INTERVALS;
}

bool RtpReceiverInfo::toBeDeleted(simtime_t now)
{
    // an RTP system should be removed from the list of known systems
    // when it hasn't been validated and hasn't been active for
    // 5 rtcp intervals or if it has been validated and has been
    // inactive for 30 minutes
    return (!isValid() && !isActive()) || (isValid() && !isActive() && (now - _startOfInactivity > 60.0 * 30.0));
}

} // namespace rtp
} // namespace inet

