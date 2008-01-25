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

#include <omnetpp.h>

#include "types.h"
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
    // rtcpLength can be calculated with cPacket::length()

    // RTCP header length size is 4 bytes
    // not all rtcp packets (in particular RTCPSDESPacket) have
    // the ssrc identifier stored in the header
    setLength(4);
};


RTCPPacket::RTCPPacket(const RTCPPacket& rtcpPacket) : cPacket() {
    setName(rtcpPacket.name());
    operator=(rtcpPacket);
};


RTCPPacket::~RTCPPacket() {
};


RTCPPacket& RTCPPacket::operator=(const RTCPPacket& rtcpPacket) {
    cPacket::operator=(rtcpPacket);
    setName(rtcpPacket.name());
    _version = rtcpPacket._version;
    _padding = rtcpPacket._padding;
    _count = rtcpPacket._count;
    _packetType = rtcpPacket._packetType;
    return *this;
};


const char *RTCPPacket::className() const {
    return "RTCPPacket";
};


cObject *RTCPPacket::dup() const {
    return new RTCPPacket(*this);
};


std::string RTCPPacket::info() {
    std::stringstream out;
    out << "RTCPPacket.packetType=" << _packetType;
    return out.str();
};


void RTCPPacket::writeContents(std::ostream& os) const {
    os << "RTCPPacket:" << endl;
    os << "  version = " << _version << endl;
    os << "  padding = " << _padding << endl;
    os << "  count = " << _count << endl;
    os << "  packetType = " << _packetType << endl;
    os << "  rtcpLength = " << rtcpLength() << endl;
};


int RTCPPacket::version() {
    return _version;
};


int RTCPPacket::padding() {
    return _padding;
};


int RTCPPacket::count() {
    return _count;
};


RTCPPacket::RTCP_PACKET_TYPE RTCPPacket::packetType() {
    return _packetType;
};


int RTCPPacket::rtcpLength() const {
    // rtcpLength is the header field length
    // of an rtcp packet
    // in 32 bit words minus one
    return (int)(length() / 4) - 1;
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
    addLength(4);
};


RTCPReceiverReportPacket::RTCPReceiverReportPacket(const RTCPReceiverReportPacket& rtcpReceiverReportPacket) : RTCPPacket() {
    setName(rtcpReceiverReportPacket.name());
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


cObject *RTCPReceiverReportPacket::dup() const {
    return new RTCPReceiverReportPacket(*this);
};


const char *RTCPReceiverReportPacket::className() const {
    return "RTCPReceiverReportPacket";
};


std::string RTCPReceiverReportPacket::info() {
    std::stringstream out;
    out << "RTCPReceiverReportPacket #rr=" << _count;
    return out.str();
};


void RTCPReceiverReportPacket::writeContents(std::ostream& os) const {
    os << "RTCPReceiverReportPacket:" << endl;
    for (int i = 0; i < _receptionReports->items(); i++) {
        if (_receptionReports->exist(i)) {
            ReceptionReport *rr = (ReceptionReport *)(_receptionReports->get(i));
            rr->writeContents(os);
        };
    };
};


u_int32 RTCPReceiverReportPacket::ssrc() {
    return _ssrc;
};


void RTCPReceiverReportPacket::setSSRC(u_int32 ssrc) {
    _ssrc = ssrc;
};


void RTCPReceiverReportPacket::addReceptionReport(ReceptionReport *report) {
    _receptionReports->add(report);
    _count++;
    // an rtcp receiver report is 24 bytes long
    addLength(24);
};


cArray *RTCPReceiverReportPacket::receptionReports() {
    return new cArray(*_receptionReports);
};



//
// RTCPSenderReportPacket
//

Register_Class(RTCPSenderReportPacket);


RTCPSenderReportPacket::RTCPSenderReportPacket(const char *name) : RTCPReceiverReportPacket(name) {
    _packetType = RTCP_PT_SR;
    _senderReport = new SenderReport("SenderReport");
    // a sender report is 20 bytes long
    addLength(20);
};


RTCPSenderReportPacket::RTCPSenderReportPacket(const RTCPSenderReportPacket& rtcpSenderReportPacket) : RTCPReceiverReportPacket() {
    setName(rtcpSenderReportPacket.name());
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


cObject *RTCPSenderReportPacket::dup() const {
    return new RTCPSenderReportPacket(*this);
};


const char *RTCPSenderReportPacket::className() const {
    return "RTCPSenderReportPacket";
};


std::string RTCPSenderReportPacket::info() {
    std::stringstream out;
    out << "RTCPSenderReportPacket.ssrc=" << _ssrc;
    return out.str();
};


void RTCPSenderReportPacket::writeContents(std::ostream& os) const {
    os << "RTCPSenderReportPacket:" << endl;
    _senderReport->writeContents(os);
    for (int i = 0; i < _receptionReports->items(); i++) {
        if (_receptionReports->exist(i)) {
            _receptionReports->get(i)->writeContents(os);
        };
    };
};


SenderReport *RTCPSenderReportPacket::senderReport() {
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
    // no addLength() needed, sdes chunks
    // directly follow the standard rtcp
    // header
};


RTCPSDESPacket::RTCPSDESPacket(const RTCPSDESPacket& rtcpSDESPacket) : RTCPPacket() {
    setName(rtcpSDESPacket.name());
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


cObject *RTCPSDESPacket::dup() const {
    return new RTCPSDESPacket(*this);
};


const char *RTCPSDESPacket::className() const {
    return "RTCPSDESPacket";
};


std::string RTCPSDESPacket::info() {
    std::stringstream out;
    out << "RTCPSDESPacket: number of sdes chunks=" << _sdesChunks->items();
    return out.str();
};


void RTCPSDESPacket::writeContents(std::ostream& os) const {
    os << "RTCPSDESPacket:" << endl;
    for (int i = 0; i < _sdesChunks->items(); i++) {
        if (_sdesChunks->exist(i))
            (*_sdesChunks)[i]->writeContents(os);
    }
};


cArray *RTCPSDESPacket::sdesChunks() {
    return new cArray(*_sdesChunks);
};


void RTCPSDESPacket::addSDESChunk(SDESChunk *sdesChunk) {
    _sdesChunks->add(sdesChunk);
    _count++;
    // the size of the rtcp packet increases by the
    // size of the sdes chunk (including ssrc)
    addLength(sdesChunk->length());
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
    addLength(4);
};


RTCPByePacket::RTCPByePacket(const RTCPByePacket& rtcpByePacket) : RTCPPacket() {
    setName(rtcpByePacket.name());
    operator=(rtcpByePacket);
};


RTCPByePacket::~RTCPByePacket() {

};


RTCPByePacket& RTCPByePacket::operator=(const RTCPByePacket& rtcpByePacket) {
    RTCPPacket::operator=(rtcpByePacket);
    _ssrc = rtcpByePacket._ssrc;
    return *this;
};


cObject *RTCPByePacket::dup() const {
    return new RTCPByePacket(*this);
};


const char *RTCPByePacket::className() const {
    return "RTCPByePacket";
};


u_int32 RTCPByePacket::ssrc() {
    return _ssrc;
};


void RTCPByePacket::setSSRC(u_int32 ssrc) {
    _ssrc = ssrc;
};


//
// RTCPCompoundPacket
//

Register_Class(RTCPCompoundPacket);


RTCPCompoundPacket::RTCPCompoundPacket(const char *name) : cPacket(name) {
    _rtcpPackets = new cArray("RTCPPackets");
    // an empty rtcp compound packet has length 0 bytes
    setLength(0);
};


RTCPCompoundPacket::RTCPCompoundPacket(const RTCPCompoundPacket& rtcpCompoundPacket) : cPacket() {
    setName(rtcpCompoundPacket.name());
    operator=(rtcpCompoundPacket);
};


RTCPCompoundPacket::~RTCPCompoundPacket() {
    delete _rtcpPackets;
};


RTCPCompoundPacket& RTCPCompoundPacket::operator=(const RTCPCompoundPacket& rtcpCompoundPacket) {
    cPacket::operator=(rtcpCompoundPacket);
    setLength(rtcpCompoundPacket.length());
    _rtcpPackets = new cArray(*(rtcpCompoundPacket._rtcpPackets));
    return *this;
};


cObject *RTCPCompoundPacket::dup() const {
    return new RTCPCompoundPacket(*this);
};


const char *RTCPCompoundPacket::className() const {
    return "RTCPCompoundPacket";
};


std::string RTCPCompoundPacket::info() {
    std::stringstream out;
    out << "RTCPCompoundPacket: number of rtcp packets=" << _rtcpPackets->items();
    return out.str();
};


void RTCPCompoundPacket::writeContents(std::ostream& os) const {
    os << "RTCPCompoundPacket:" << endl;
    for (int i = 0; i < _rtcpPackets->items(); i++) {
        if (_rtcpPackets->exist(i)) {
            _rtcpPackets->get(i)->writeContents(os);
        }
    }
};


void RTCPCompoundPacket::addRTCPPacket(RTCPPacket *rtcpPacket) {
    //rtcpPacket->setOwner(_rtcpPackets);
    _rtcpPackets->add(rtcpPacket);
    // the size of the rtcp compound packet increases
    // by the size of the added rtcp packet
    addLength(rtcpPacket->length());
};


cArray *RTCPCompoundPacket::rtcpPackets() {
    return new cArray(*_rtcpPackets);
}
