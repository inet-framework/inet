/***************************************************************************
                          RTPSenderInfo.cc  -  description
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


/** \file RTPSenderInfo.cc
 * This file contains the implementation of member functions of the class RTPSenderInfo.
 */

#include <omnetpp.h>

#include "types.h"
#include "RTPSenderInfo.h"


Register_Class(RTPSenderInfo);

RTPSenderInfo::RTPSenderInfo(u_int32 ssrc) : RTPParticipantInfo(ssrc) {
    _startTime = 0.0;
    _clockRate = 0;
    _timeStampBase = 0;
    _sequenceNumberBase = 0;
    _packetsSent = 0;
    _bytesSent = 0;

};


RTPSenderInfo::RTPSenderInfo(const RTPSenderInfo& senderInfo) : RTPParticipantInfo() {
    setName(senderInfo.name());
    operator=(senderInfo);
};


RTPSenderInfo::~RTPSenderInfo() {

};


RTPSenderInfo& RTPSenderInfo::operator=(const RTPSenderInfo& senderInfo) {
    RTPParticipantInfo::operator=(senderInfo);
    _startTime = senderInfo._startTime;
    _clockRate = senderInfo._clockRate;
    _timeStampBase = senderInfo._timeStampBase;
    _sequenceNumberBase = senderInfo._sequenceNumberBase;
    _packetsSent = senderInfo._packetsSent;
    _bytesSent = senderInfo._bytesSent;
    return *this;
};


cObject *RTPSenderInfo::dup() const {
    return new RTPSenderInfo(*this);
};


const char *RTPSenderInfo::className() const {
    return "RTPSenderInfo";
};


void RTPSenderInfo::processRTPPacket(RTPPacket *packet, simtime_t arrivalTime) {
    _packetsSent++;
    _bytesSent = _bytesSent + packet->payloadLength();

    // call corresponding method of superclass
    // for setting _silentIntervals
    // it deletes the packet !!!
    RTPParticipantInfo::processRTPPacket(packet, arrivalTime);
};


void RTPSenderInfo::processReceptionReport(ReceptionReport *report, simtime_t arrivalTime) {
    delete report;
};


SenderReport *RTPSenderInfo::senderReport(simtime_t now) {
    if (isSender()) {
        SenderReport *senderReport = new SenderReport("SenderReport");
        // ntp time stamp is 64 bit integer

        u_int64 ntpSeconds = (u_int64)now;
        u_int64 ntpFraction = (u_int64)((now - (simtime_t)ntpSeconds) * 65536.0 * 65536.0);

        senderReport->setNTPTimeStamp((u_int64)(ntpSeconds << 32) + ntpFraction);
        senderReport->setRTPTimeStamp((now - _startTime) * _clockRate);
        senderReport->setPacketCount(_packetsSent);
        senderReport->setByteCount(_bytesSent);
        return senderReport;
    }
    else {
        return NULL;
    };
};


void RTPSenderInfo::setStartTime(simtime_t startTime) {
    _startTime = startTime;
};


void RTPSenderInfo::setClockRate(int clockRate) {
    _clockRate = clockRate;
};


void RTPSenderInfo::setTimeStampBase(u_int32 timeStampBase) {
    _timeStampBase = timeStampBase;
};


void RTPSenderInfo::setSequenceNumberBase(u_int16 sequenceNumberBase) {
    _sequenceNumberBase = sequenceNumberBase;
};


bool RTPSenderInfo::toBeDeleted(simtime_t now) {
    return false;
}
