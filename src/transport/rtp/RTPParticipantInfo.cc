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

#include "RTPParticipantInfo.h"
#include "reports.h"


Register_Class(RTPParticipantInfo);


RTPParticipantInfo::RTPParticipantInfo(uint32 ssrc) : cObject()
{
    _sdesChunk = new SDESChunk("SDESChunk", ssrc);
    // because there haven't been sent any rtp packets
    // by this endsystem at all, the number of silent
    // intervals would be undefined; to calculate with
    // it but not to regard this endsystem as a sender
    // it is set to 3; see isSender() for details
    _silentIntervals = 3;
    _address = IPAddress::UNSPECIFIED_ADDRESS;
    _rtpPort = PORT_UNDEF;
    _rtcpPort = PORT_UNDEF;
}


RTPParticipantInfo::RTPParticipantInfo(const RTPParticipantInfo& participantInfo) : cObject()
{
    operator=(participantInfo);
}


RTPParticipantInfo::~RTPParticipantInfo()
{
    delete _sdesChunk;
}


RTPParticipantInfo& RTPParticipantInfo::operator=(const RTPParticipantInfo& participantInfo)
{
    cObject::operator=(participantInfo);
    _sdesChunk = new SDESChunk(*(participantInfo._sdesChunk));
    _address = participantInfo._address;
    _rtpPort = participantInfo._rtpPort;
    _rtcpPort = participantInfo._rtcpPort;
    return *this;
}


RTPParticipantInfo *RTPParticipantInfo::dup() const
{
    return new RTPParticipantInfo(*this);
}


void RTPParticipantInfo::processRTPPacket(RTPPacket *packet, int id, simtime_t arrivalTime)
{
    _silentIntervals = 0;
    delete packet;
}


void RTPParticipantInfo::processSenderReport(SenderReport *report, simtime_t arrivalTime)
{
    // useful code can be found in subclasses
    delete report;
}


void RTPParticipantInfo::processReceptionReport(ReceptionReport *report, simtime_t arrivalTime)
{
    // useful code can be found in subclasses
    delete report;
}


void RTPParticipantInfo::processSDESChunk(SDESChunk *sdesChunk, simtime_t arrivalTime)
{
    for (int i = 0; i < sdesChunk->size(); i++) {
        if (sdesChunk->exist(i)) {
            SDESItem *sdesItem = (SDESItem *)(sdesChunk->remove(i));
            addSDESItem(sdesItem);
        }
    }
    delete sdesChunk;
}


SDESChunk *RTPParticipantInfo::getSDESChunk()
{
    return new SDESChunk(*_sdesChunk);
}


void RTPParticipantInfo::addSDESItem(SDESItem *sdesItem)
{
    _sdesChunk->addSDESItem(sdesItem);
}


bool RTPParticipantInfo::isSender()
{
    return (_silentIntervals <= 1);
}


ReceptionReport *RTPParticipantInfo::receptionReport(simtime_t now)
{
    return NULL;
}


SenderReport *RTPParticipantInfo::senderReport(simtime_t now)
{
    return NULL;
}


void RTPParticipantInfo::nextInterval(simtime_t now)
{
    _silentIntervals++;
}


bool RTPParticipantInfo::toBeDeleted(simtime_t now)
{
    return false;
}


uint32 RTPParticipantInfo::getSSRC()
{
    return _sdesChunk->getSSRC();
}


void RTPParticipantInfo::setSSRC(uint32 ssrc)
{
    _sdesChunk->setSSRC(ssrc);
}


void RTPParticipantInfo::addSDESItem(SDESItem::SDES_ITEM_TYPE type, const char *content)
{
    _sdesChunk->addSDESItem(new SDESItem(type, content));
}


IPAddress RTPParticipantInfo::getAddress()
{
    return _address;
}


void RTPParticipantInfo::setAddress(IPAddress address)
{
    _address = address;
}


int RTPParticipantInfo::getRTPPort()
{
    return _rtpPort;
}


void RTPParticipantInfo::setRTPPort(int rtpPort)
{
    _rtpPort = rtpPort;
}


int RTPParticipantInfo::getRTCPPort()
{
    return _rtcpPort;
}


void RTPParticipantInfo::setRTCPPort(int rtcpPort)
{
    _rtcpPort = rtcpPort;
}


char *RTPParticipantInfo::ssrcToName(uint32 ssrc)
{
    char name[9];
    sprintf(name, "%08x", ssrc);
    return opp_strdup(name);
}


void RTPParticipantInfo::dump() const
{
    std::cout <<" adress= "<< _address
              <<" rtpPort= "<< _rtpPort
              <<" rtcpPort= "<< _rtcpPort
              << endl;
}
