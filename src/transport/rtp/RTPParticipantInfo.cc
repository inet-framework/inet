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


#include "RTPParticipantInfo.h"

#include "reports.h"


Register_Class(RTPParticipantInfo);


RTPParticipantInfo::RTPParticipantInfo(uint32 ssrc)
  : cObject(), _sdesChunk("SDESChunk", ssrc)
{
    // because there haven't been sent any RTP packets
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
}

RTPParticipantInfo& RTPParticipantInfo::operator=(const RTPParticipantInfo& participantInfo)
{
    cObject::operator=(participantInfo);
    _sdesChunk = participantInfo._sdesChunk;
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


void RTPParticipantInfo::processSenderReport(SenderReport &report, simtime_t arrivalTime)
{
    // useful code can be found in subclasses
}

void RTPParticipantInfo::processReceptionReport(ReceptionReport &report, simtime_t arrivalTime)
{
    // useful code can be found in subclasses
}

void RTPParticipantInfo::processSDESChunk(SDESChunk *sdesChunk, simtime_t arrivalTime)
{
    for (int i = 0; i < sdesChunk->size(); i++)
    {
        if (sdesChunk->exist(i))
        {
            SDESItem *sdesItem = (SDESItem *)(sdesChunk->remove(i));
            addSDESItem(sdesItem);
        }
    }
    delete sdesChunk;
}

SDESChunk *RTPParticipantInfo::getSDESChunk() const
{
    return new SDESChunk(_sdesChunk);
}

void RTPParticipantInfo::addSDESItem(SDESItem *sdesItem)
{
    _sdesChunk.addSDESItem(sdesItem);
}

bool RTPParticipantInfo::isSender() const
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

uint32 RTPParticipantInfo::getSsrc() const
{
    return _sdesChunk.getSsrc();
}

void RTPParticipantInfo::setSsrc(uint32 ssrc)
{
    _sdesChunk.setSsrc(ssrc);
}

void RTPParticipantInfo::addSDESItem(SDESItem::SDES_ITEM_TYPE type, const char *content)
{
    _sdesChunk.addSDESItem(new SDESItem(type, content));
}

IPAddress RTPParticipantInfo::getAddress() const
{
    return _address;
}

void RTPParticipantInfo::setAddress(IPAddress address)
{
    _address = address;
}


int RTPParticipantInfo::getRTPPort() const
{
    return _rtpPort;
}


void RTPParticipantInfo::setRTPPort(int rtpPort)
{
    _rtpPort = rtpPort;
}


int RTPParticipantInfo::getRTCPPort() const
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
