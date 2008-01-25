/***************************************************************************
                          RTPReceiverInfo.cc  -  description
                             -------------------
    begin                : Wed Dec 5 2001
    copyright            : (C) 2001 by Matthias Oppitz
    email                : Matthias.Oppitz@gmx.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


/** \file RTPReceiverInfo.cc
 * This file contains the implementation of member functions of the class RTPReceiverInfo.
 */

#include <omnetpp.h>

#include "types.h"
#include "RTPReceiverInfo.h"


Register_Class(RTPReceiverInfo);

RTPReceiverInfo::RTPReceiverInfo(u_int32 ssrc) : RTPParticipantInfo(ssrc)  {

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
};


RTPReceiverInfo::RTPReceiverInfo(const RTPReceiverInfo& receiverInfo) : RTPParticipantInfo() {
    setName(receiverInfo.name());
    operator=(receiverInfo);
};


RTPReceiverInfo::~RTPReceiverInfo() {
}


RTPReceiverInfo& RTPReceiverInfo::operator=(const RTPReceiverInfo& receiverInfo) {
    RTPParticipantInfo::operator=(receiverInfo);

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

    return *this;
};


cObject *RTPReceiverInfo::dup() const {
    return new RTPReceiverInfo(*this);
};


const char *RTPReceiverInfo::className() const {
    return "RTPReceiverInfo";
};


void RTPReceiverInfo::processRTPPacket(RTPPacket *packet, simtime_t arrivalTime) {

    // this endsystem sends, it isn't inactive
    _inactiveIntervals = 0;

    _packetsReceived++;
    _itemsReceived++;

    if (_packetsReceived == 1) {
        _sequenceNumberBase = packet->sequenceNumber();
    }
    else {
        if (packet->sequenceNumber() > _highestSequenceNumber) {
            // it is possible that this is a late packet from the
            // previous sequence wrap
            if (!(packet->sequenceNumber() > 0xFFEF && _highestSequenceNumber < 0x10))
                _highestSequenceNumber = packet->sequenceNumber();
        }
        else {
            // is it a sequence number wrap around 0xFFFF to 0x0000 ?
            if (packet->sequenceNumber() < 0x10 && _highestSequenceNumber > 0xFFEF) {
                _sequenceNumberCycles += 0x00010000;
                _highestSequenceNumber = packet->sequenceNumber();
            }
        };
        // calculate interarrival jitter
        if (_clockRate != 0) {
            double d = (double)(packet->timeStamp() - _lastPacketRTPTimeStamp) - (arrivalTime - _lastPacketArrivalTime) * (double)_clockRate;
            if (d < 0)
                d = -d;
            _jitter = _jitter + (d - _jitter) / 16;
        };
        _lastPacketRTPTimeStamp = packet->timeStamp();
        _lastPacketArrivalTime = arrivalTime;
    };



    RTPParticipantInfo::processRTPPacket(packet, arrivalTime);

};


void RTPReceiverInfo::processSenderReport(SenderReport *report, simtime_t arrivalTime) {

    _lastSenderReportArrivalTime = arrivalTime;
    if (_lastSenderReportRTPTimeStamp == 0) {
        _lastSenderReportRTPTimeStamp = report->rtpTimeStamp();
        _lastSenderReportNTPTimeStamp = report->ntpTimeStamp();
    }
    else if (_clockRate == 0) {
        u_int32 rtpTicks = report->rtpTimeStamp() - _lastSenderReportRTPTimeStamp;
        u_int64 ntpDifference = report->ntpTimeStamp() - _lastSenderReportNTPTimeStamp;
        long double ntpSeconds = (long double)ntpDifference / (long double)(0xFFFFFFFF);
        _clockRate = (int)((long double)rtpTicks / ntpSeconds);
    }

    _inactiveIntervals = 0;
    _itemsReceived++;

    delete report;
};


void RTPReceiverInfo::processSDESChunk(SDESChunk *sdesChunk, simtime_t arrivalTime) {
    RTPParticipantInfo::processSDESChunk(sdesChunk, arrivalTime);
    _itemsReceived++;
    _inactiveIntervals = 0;
};


ReceptionReport *RTPReceiverInfo::receptionReport(simtime_t now) {

    if (isSender()) {
        ReceptionReport *receptionReport = new ReceptionReport("ReceiverReport");
        receptionReport->setSSRC(ssrc());

        u_int64 packetsExpected = _sequenceNumberCycles + (u_int64)_highestSequenceNumber - (u_int64)_sequenceNumberBase + (u_int64)1;
        u_int64 packetsLost = packetsExpected - _packetsReceived;

        int32 packetsExpectedInInterval = _sequenceNumberCycles + _highestSequenceNumber - _highestSequenceNumberPrior;
        int32 packetsReceivedInInterval = _packetsReceived - _packetsReceivedPrior;
        int32 packetsLostInInterval = packetsExpectedInInterval - packetsReceivedInInterval;
        u_int8 fractionLost = 0;
        if (packetsLostInInterval > 0) {
            fractionLost = (packetsLostInInterval << 8) / packetsExpectedInInterval;
        };

        receptionReport->setFractionLost(fractionLost);
        receptionReport->setPacketsLostCumulative(packetsLost);
        receptionReport->setSequenceNumber(_sequenceNumberCycles + _highestSequenceNumber);

        receptionReport->setJitter((u_int32)_jitter);

        // the middle 32 bit of the ntp time stamp of the last sender report
        receptionReport->setLastSR((_lastSenderReportNTPTimeStamp >> 16) & 0xFFFFFFFF);

        // the delay since the arrival of the last sender report in units
        // of 1 / 65536 seconds
        // 0 if no sender report has ben received

        receptionReport->setDelaySinceLastSR(_lastSenderReportArrivalTime == 0.0 ? 0 : (u_int32)((now - _lastSenderReportArrivalTime) * 65536.0));

        return receptionReport;
    }
    else
        return NULL;
};


void RTPReceiverInfo::nextInterval(simtime_t now) {
    _inactiveIntervals++;
    if (_inactiveIntervals == 5) {
        _startOfInactivity = now;
    }
    _highestSequenceNumberPrior = _highestSequenceNumber + _sequenceNumberCycles;
    _packetsReceivedPrior = _packetsReceived;
    RTPParticipantInfo::nextInterval(now);
};


bool RTPReceiverInfo::active() {
    return (_inactiveIntervals < 5);
};


bool RTPReceiverInfo::valid() {
    return (_itemsReceived >= 5);
};


bool RTPReceiverInfo::toBeDeleted(simtime_t now) {
    // an rtp system should be removed from the list of known systems
    // when it hasn't been validated and hasn't been active for
    // 5 rtcp intervals or if it has been validated and has been
    // inactive for 30 minutes
    return (!valid() && !active()) || (valid() && !active() && (now - _startOfInactivity > 60.0 * 30.0));
};
