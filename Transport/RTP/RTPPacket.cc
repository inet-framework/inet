/***************************************************************************
                          RTPPacket.cc  -  description
                             -------------------
    begin                : Mon Oct 22 2001
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

/** \file RTPPacket.cc
 * This file contains the implementaion of member functions of the class RTPPacket.
 */

#include <omnetpp.h>
#include "types.h"
#include "RTPPacket.h"


Register_Class(RTPPacket);


RTPPacket::RTPPacket(const char *name) : cPacket(name) {
    _version = 2;
    _padding = 0;
    _extension = 0;
    _csrcCount = 0;
    _marker = 0;
    _payloadType = 0;
    _sequenceNumber = 0;
    _timeStamp = 0;
    _ssrc = 0;

    // a standard rtp packet without csrcs and data has a length of 12 bytes
    setLength(fixedHeaderLength());
};


RTPPacket::RTPPacket(const RTPPacket& packet) : cPacket() {
    setName(packet.name());
    operator=(packet);
};


RTPPacket::~RTPPacket() {
    // when csrcList is implemented this
    // should free the memory used for it
};


cObject *RTPPacket::dup() const {
    return new RTPPacket(*this);
};


RTPPacket& RTPPacket::operator=(const RTPPacket& packet) {
    cPacket::operator=(packet);
    _version = packet._version;
    _padding = packet._padding;
    _extension = packet._extension;
    _csrcCount = packet._csrcCount;
    _marker = packet._marker;
    _payloadType = packet._payloadType;
    _sequenceNumber = packet._sequenceNumber;
    _timeStamp = packet._timeStamp;
    _ssrc = packet._ssrc;
    return *this;
};


const char *RTPPacket::className() const {
    return "RTPPacket";
};


std::string RTPPacket::info() {
    std::stringstream out;
    out << "RTPPacket: payloadType=" << _payloadType << " payloadLength=" << payloadLength();
    return out.str();
};


void RTPPacket::writeContents(std::ostream& os) {
    os << "RTPPacket:" << endl;
    os << "  payloadType = " << _payloadType << endl;
    os << "  sequenceNumber = " << _sequenceNumber << endl;
    os << "  timeStamp = " << _timeStamp << endl;
    os << "  payloadLength = " << payloadLength() << endl;
};


int RTPPacket::marker() {
    return _marker;
};


void RTPPacket::setMarker(int marker) {
    _marker = marker;
};


int RTPPacket::payloadType() {
    return _payloadType;
};


void RTPPacket::setPayloadType(int payloadType) {
    _payloadType = payloadType;
};


u_int16 RTPPacket::sequenceNumber() {
    return _sequenceNumber;
};


void RTPPacket::setSequenceNumber(u_int16 sequenceNumber) {
    _sequenceNumber = sequenceNumber;
};


u_int32 RTPPacket::timeStamp() {
    return _timeStamp;
};


void RTPPacket::setTimeStamp(u_int32 timeStamp) {
    _timeStamp = timeStamp;
};


u_int32 RTPPacket::ssrc() {
    return _ssrc;
};


void RTPPacket::setSSRC(u_int32 ssrc) {
    _ssrc = ssrc;
};

int RTPPacket::fixedHeaderLength() {
    return 12;
};

int RTPPacket::headerLength() {
    // fixed header is 12 bytes long,
    // add 4 bytes for every csrc identifier
    return(fixedHeaderLength() + 4 * _csrcCount);
};


int RTPPacket::payloadLength() {
    return(length() - headerLength());
};


int RTPPacket::compareFunction(cObject *packet1, cObject *packet2) {
    return ((RTPPacket *)packet1)->sequenceNumber() - ((RTPPacket *)packet2)->sequenceNumber();
};
