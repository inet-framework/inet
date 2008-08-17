/***************************************************************************
                          RTCPacket.cc  -  description
                             -------------------
    begin                : Sun Oct 21 2001
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

/** \file RTCPPacket.cc
 * In this file member functions of the rtcp classes RTCPPacket,
 * RTCPReceiverReportPacket, RTCPSenderReportPacket, RTCPSDESPacket,
 * RTCPByePacket and RTCPCompoundPacket are implemented.
 */

#include "RTCPPacket.h"
#include "reports.h"
#include "sdes.h"

//
// RTCPPacket
//

// register class RTCPPacket for omnet++
Register_Class(RTCPPacket);


RTCPPacket::RTCPPacket(const char *name) : cPacket(name) {
    // initialize variables
    _version = 2;
    _padding = 0;
    _count = 0;
    _packetType = RTCP_PT_UNDEF;
    // rtcpLength can be calculated with cPacket::getLength()

    // RTCP header length size is 4 bytes
    // not all rtcp packets (in particular RTCPSDESPacket) have
    // the ssrc identifier stored in the header
    setByteLength(4);
};


RTCPPacket::RTCPPacket(const RTCPPacket& rtcpPacket) : cPacket() {
    setName(rtcpPacket.getName());
    operator=(rtcpPacket);
};


RTCPPacket::~RTCPPacket() {
};


RTCPPacket& RTCPPacket::operator=(const RTCPPacket& rtcpPacket) {
    cPacket::operator=(rtcpPacket);
    setName(rtcpPacket.getName());
    _version = rtcpPacket._version;
    _padding = rtcpPacket._padding;
    _count = rtcpPacket._count;
    _packetType = rtcpPacket._packetType;
    return *this;
};


RTCPPacket *RTCPPacket::dup() const {
    return new RTCPPacket(*this);
};


std::string RTCPPacket::info() {
    std::stringstream out;
    out << "RTCPPacket.packetType=" << _packetType;
    return out.str();
};


void RTCPPacket::dump(std::ostream& os) const {
    os << "RTCPPacket:" << endl;
    os << "  version = " << _version << endl;
    os << "  padding = " << _padding << endl;
    os << "  count = " << _count << endl;
    os << "  packetType = " << _packetType << endl;
    os << "  rtcpLength = " << getRtcpLength() << endl;
};


int RTCPPacket::getVersion() {
    return _version;
};


int RTCPPacket::getPadding() {
    return _padding;
};


int RTCPPacket::getCount() {
    return _count;
};


RTCPPacket::RTCP_PACKET_TYPE RTCPPacket::getPacketType() {
    return _packetType;
};


int RTCPPacket::getRtcpLength() const {
    // rtcpLength is the header field length
    // of an rtcp packet
    // in 32 bit words minus one
    return (int)(getByteLength() / 4) - 1;
};


//
// RTCPReceiverReportPacket
//

Register_Class(RTCPReceiverReportPacket);


RTCPReceiverReportPacket::RTCPReceiverReportPacket(const char *name) : RTCPPacket(name) {
    _packetType = RTCP_PT_RR;
    _ssrc = 0;
    _receptionReports = new cArray("ReceptionReports");
    // an empty rtcp receiver report packet is 4 bytes
    // longer, the ssrc identifier is stored in it
    addByteLength(4);
};

RTCPReceiverReportPacket::RTCPReceiverReportPacket(const RTCPReceiverReportPacket& rtcpReceiverReportPacket) : RTCPPacket() {
    _receptionReports = NULL;
    setName(rtcpReceiverReportPacket.getName());
    operator=(rtcpReceiverReportPacket);
};


RTCPReceiverReportPacket::~RTCPReceiverReportPacket() {
    delete _receptionReports;
};


RTCPReceiverReportPacket& RTCPReceiverReportPacket::operator=(const RTCPReceiverReportPacket& rtcpReceiverReportPacket) {
    RTCPPacket::operator=(rtcpReceiverReportPacket);
    _ssrc = rtcpReceiverReportPacket._ssrc;
    _receptionReports = new cArray(*(rtcpReceiverReportPacket._receptionReports));
    return *this;
};


RTCPReceiverReportPacket *RTCPReceiverReportPacket::dup() const {
    return new RTCPReceiverReportPacket(*this);
};


std::string RTCPReceiverReportPacket::info() {
    std::stringstream out;
    out << "RTCPReceiverReportPacket #rr=" << _count;
    return out.str();
};


void RTCPReceiverReportPacket::dump(std::ostream& os) const {
    os << "RTCPReceiverReportPacket:" << endl;
    for (int i = 0; i < _receptionReports->size(); i++) {
        if (_receptionReports->exist(i)) {
            ReceptionReport *rr = (ReceptionReport *)(_receptionReports->get(i));
            rr->dump(os);
        };
    };
};


uint32 RTCPReceiverReportPacket::getSSRC() {
    return _ssrc;
};


void RTCPReceiverReportPacket::setSSRC(uint32 ssrc) {
    _ssrc = ssrc;
};


void RTCPReceiverReportPacket::addReceptionReport(ReceptionReport *report) {
    _receptionReports->add(report);
    _count++;
    // an rtcp receiver report is 24 bytes long
    addByteLength(24);
};


cArray *RTCPReceiverReportPacket::getReceptionReports() {
    return new cArray(*_receptionReports);
};



//
// RTCPSenderReportPacket
//

Register_Class(RTCPSenderReportPacket);


RTCPSenderReportPacket::RTCPSenderReportPacket(const char *name) : RTCPReceiverReportPacket(name) {
    _packetType = RTCP_PT_SR;
    _senderReport = new SenderReport();
    // a sender report is 20 bytes long
    addByteLength(20);
};


RTCPSenderReportPacket::RTCPSenderReportPacket(const RTCPSenderReportPacket& rtcpSenderReportPacket) : RTCPReceiverReportPacket() {
    setName(rtcpSenderReportPacket.getName());
    operator=(rtcpSenderReportPacket);
};


RTCPSenderReportPacket::~RTCPSenderReportPacket() {
    delete _senderReport;
};


RTCPSenderReportPacket& RTCPSenderReportPacket::operator=(const RTCPSenderReportPacket& rtcpSenderReportPacket) {
    RTCPReceiverReportPacket::operator=(rtcpSenderReportPacket);
    _senderReport = new SenderReport(*(rtcpSenderReportPacket._senderReport));
    return *this;
};


RTCPSenderReportPacket *RTCPSenderReportPacket::dup() const {
    return new RTCPSenderReportPacket(*this);
};


std::string RTCPSenderReportPacket::info() {
    std::stringstream out;
    out << "RTCPSenderReportPacket.ssrc=" << _ssrc;
    return out.str();
};


void RTCPSenderReportPacket::dump(std::ostream& os) const {
    os << "RTCPSenderReportPacket:" << endl;
    _senderReport->dump(os);
    for (int i = 0; i < _receptionReports->size(); i++) {
        if (_receptionReports->exist(i)) {
            //FIXME _receptionReports->get(i)->dump(os);
        };
    };
};


SenderReport *RTCPSenderReportPacket::getSenderReport() {
    return new SenderReport(*_senderReport);
};


void RTCPSenderReportPacket::setSenderReport(SenderReport *report) {
    delete _senderReport;
    _senderReport = report;
};



//
// RTCPSDESPacket
//

Register_Class(RTCPSDESPacket);


RTCPSDESPacket::RTCPSDESPacket(const char *name) : RTCPPacket(name) {
    _packetType = RTCP_PT_SDES;
    _sdesChunks = new cArray("SDESChunks");
    // no addByteLength() needed, sdes chunks
    // directly follow the standard rtcp
    // header
};


RTCPSDESPacket::RTCPSDESPacket(const RTCPSDESPacket& rtcpSDESPacket) : RTCPPacket() {
    setName(rtcpSDESPacket.getName());
    operator=(rtcpSDESPacket);
};


RTCPSDESPacket::~RTCPSDESPacket() {
    delete _sdesChunks;
};


RTCPSDESPacket& RTCPSDESPacket::operator=(const RTCPSDESPacket& rtcpSDESPacket) {
    RTCPPacket::operator=(rtcpSDESPacket);
    _sdesChunks = new cArray(*(rtcpSDESPacket._sdesChunks));
    return *this;
};


RTCPSDESPacket *RTCPSDESPacket::dup() const {
    return new RTCPSDESPacket(*this);
};


std::string RTCPSDESPacket::info() {
    std::stringstream out;
    out << "RTCPSDESPacket: number of sdes chunks=" << _sdesChunks->size();
    return out.str();
};


void RTCPSDESPacket::dump(std::ostream& os) const {
    os << "RTCPSDESPacket:" << endl;
    for (int i = 0; i < _sdesChunks->size(); i++) {
        if (_sdesChunks->exist(i))
            ;//FIXME (*_sdesChunks)[i]->dump(os);
    }
};


cArray *RTCPSDESPacket::getSdesChunks() {
    return new cArray(*_sdesChunks);
};


void RTCPSDESPacket::addSDESChunk(SDESChunk *sdesChunk) {
    _sdesChunks->add(sdesChunk);
    _count++;
    // the size of the rtcp packet increases by the
    // size of the sdes chunk (including ssrc)
    addByteLength(sdesChunk->getLength());
};



//
// RTCPByePacket
//

Register_Class(RTCPByePacket);

RTCPByePacket::RTCPByePacket(const char *name) : RTCPPacket(name) {
    _packetType = RTCP_PT_BYE;
    _count = 1;
    _ssrc = 0;
    // space for the ssrc identifier
    addByteLength(4);
};


RTCPByePacket::RTCPByePacket(const RTCPByePacket& rtcpByePacket) : RTCPPacket() {
    setName(rtcpByePacket.getName());
    operator=(rtcpByePacket);
};


RTCPByePacket::~RTCPByePacket() {

};


RTCPByePacket& RTCPByePacket::operator=(const RTCPByePacket& rtcpByePacket) {
    RTCPPacket::operator=(rtcpByePacket);
    _ssrc = rtcpByePacket._ssrc;
    return *this;
};


RTCPByePacket *RTCPByePacket::dup() const {
    return new RTCPByePacket(*this);
};


uint32 RTCPByePacket::getSSRC() {
    return _ssrc;
};


void RTCPByePacket::setSSRC(uint32 ssrc) {
    _ssrc = ssrc;
};


//
// RTCPCompoundPacket
//

Register_Class(RTCPCompoundPacket);


RTCPCompoundPacket::RTCPCompoundPacket(const char *name) : cPacket(name) {
    _rtcpPackets = new cArray("RTCPPackets");
    // an empty rtcp compound packet has length 0 bytes
    setByteLength(0);
};


RTCPCompoundPacket::RTCPCompoundPacket(const RTCPCompoundPacket& rtcpCompoundPacket) : cPacket() {
    setName(rtcpCompoundPacket.getName());
    operator=(rtcpCompoundPacket);
};


RTCPCompoundPacket::~RTCPCompoundPacket() {
    delete _rtcpPackets;
};


RTCPCompoundPacket& RTCPCompoundPacket::operator=(const RTCPCompoundPacket& rtcpCompoundPacket) {
    cPacket::operator=(rtcpCompoundPacket);
    setByteLength(rtcpCompoundPacket.getByteLength());
    _rtcpPackets = new cArray(*(rtcpCompoundPacket._rtcpPackets));
    return *this;
};


RTCPCompoundPacket *RTCPCompoundPacket::dup() const {
    return new RTCPCompoundPacket(*this);
};


std::string RTCPCompoundPacket::info() {
    std::stringstream out;
    out << "RTCPCompoundPacket: number of rtcp packets=" << _rtcpPackets->size();
    return out.str();
};


void RTCPCompoundPacket::dump(std::ostream& os) const {
    os << "RTCPCompoundPacket:" << endl;
    for (int i = 0; i < _rtcpPackets->size(); i++) {
        if (_rtcpPackets->exist(i)) {
            //FIXME _rtcpPackets->get(i)->dump(os);
        }
    }
};


void RTCPCompoundPacket::addRTCPPacket(RTCPPacket *rtcpPacket) {
    //rtcpPacket->setOwner(_rtcpPackets);
    _rtcpPackets->add(rtcpPacket);
    // the size of the rtcp compound packet increases
    // by the size of the added rtcp packet
    addByteLength(rtcpPacket->getByteLength());
};


cArray *RTCPCompoundPacket::getRtcpPackets() {
    return new cArray(*_rtcpPackets);
}
