/***************************************************************************
                          RTPParticipantInfo.cc  -  description
                             -------------------
    begin                : Wed Oct 24 2001
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

/** \file RTPParticipantInfo.cc
 * This file contains the implementation of member functions of the class RTPParticipantInfo.
 * \sa RTPParticipantInfo
 */

#include <omnetpp.h>

#include "types.h"
#include "RTPParticipantInfo.h"
#include "reports.h"


Register_Class(RTPParticipantInfo);


RTPParticipantInfo::RTPParticipantInfo(u_int32 ssrc) : cObject() {
    setName(ssrcToName(ssrc));
    _sdesChunk = new SDESChunk("SDESChunk", ssrc);
    // because there haven't been sent any rtp packets
    // by this endsystem at all, the number of silent
    // intervals would be undefined; to calculate with
    // it but not to regard this endsystem as a sender
    // it is set to 3; see isSender() for details
    _silentIntervals = 3;
    _address = IPADDRESS_UNDEF;
    _rtpPort = IPSuite_PORT_UNDEF;
    _rtcpPort = IPSuite_PORT_UNDEF;
};


RTPParticipantInfo::RTPParticipantInfo(const RTPParticipantInfo& participantInfo) : cObject() {
    setName(participantInfo.name());
    operator=(participantInfo);
};


RTPParticipantInfo::~RTPParticipantInfo() {
    //delete _sdesChunk;
};


RTPParticipantInfo& RTPParticipantInfo::operator=(const RTPParticipantInfo& participantInfo) {
    cObject::operator=(participantInfo);
    _sdesChunk = new SDESChunk(*(participantInfo._sdesChunk));
    _address = participantInfo._address;
    _rtpPort = participantInfo._rtpPort;
    _rtcpPort = participantInfo._rtcpPort;
    return *this;
};


cObject *RTPParticipantInfo::dup() const {
    return new RTPParticipantInfo(*this);
};


const char *RTPParticipantInfo::className() const {
    return "RTPParticipantInfo";
};


void RTPParticipantInfo::processRTPPacket(RTPPacket *packet, simtime_t arrivalTime) {
    _silentIntervals = 0;
    delete packet;
};


void RTPParticipantInfo::processSenderReport(SenderReport *report, simtime_t arrivalTime) {
    // useful code can be found in subclasses
    delete report;
};


void RTPParticipantInfo::processReceptionReport(ReceptionReport *report, simtime_t arrivalTime) {
    // useful code can be found in subclasses
    delete report;
};


void RTPParticipantInfo::processSDESChunk(SDESChunk *sdesChunk, simtime_t arrivalTime) {
    for (int i = 0; i < sdesChunk->items(); i++) {
        if (sdesChunk->exist(i)) {
            SDESItem *sdesItem = (SDESItem *)(sdesChunk->remove(i));
            addSDESItem(sdesItem);
        }
    }
    delete sdesChunk;
};


SDESChunk *RTPParticipantInfo::sdesChunk() {
    return new SDESChunk(*_sdesChunk);
};


void RTPParticipantInfo::addSDESItem(SDESItem *sdesItem) {
    _sdesChunk->addSDESItem(sdesItem);
};


bool RTPParticipantInfo::isSender() {
    return (_silentIntervals <= 1);
};


ReceptionReport *RTPParticipantInfo::receptionReport(simtime_t now) {
  opp_error("Returning NULL pointer results in segmentation fault");
    return NULL;
};


SenderReport *RTPParticipantInfo::senderReport(simtime_t now) {
    return NULL;
};


void RTPParticipantInfo::nextInterval(simtime_t now) {
    _silentIntervals++;
};


bool RTPParticipantInfo::toBeDeleted(simtime_t now) {
    return false;
};


u_int32 RTPParticipantInfo::ssrc() {
    return _sdesChunk->ssrc();
};


void RTPParticipantInfo::setSSRC(u_int32 ssrc) {
    setName(ssrcToName(ssrc));
    _sdesChunk->setSSRC(ssrc);
};


void RTPParticipantInfo::addSDESItem(SDESItem::SDES_ITEM_TYPE type, const char *content) {
    _sdesChunk->addSDESItem(new SDESItem(type, content));
};


IN_Addr RTPParticipantInfo::address() {
    return _address;
};


void RTPParticipantInfo::setAddress(IN_Addr address) {
    _address = address;
};


IN_Port RTPParticipantInfo::rtpPort() {
    return _rtpPort;
};


void RTPParticipantInfo::setRTPPort(IN_Port rtpPort) {
    _rtpPort = rtpPort;
};


IN_Port RTPParticipantInfo::rtcpPort() {
    return _rtcpPort;
};


void RTPParticipantInfo::setRTCPPort(IN_Port rtcpPort) {
    _rtcpPort = rtcpPort;
};


char *RTPParticipantInfo::ssrcToName(u_int32 ssrc) {
    char name[9];
    sprintf(name, "%08x", ssrc);
    return opp_strdup(name);
};
