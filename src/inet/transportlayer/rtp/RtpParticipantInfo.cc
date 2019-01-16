/***************************************************************************
                          RtpParticipantInfo.cc  -  description
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

#include "inet/transportlayer/rtp/Reports_m.h"
#include "inet/transportlayer/rtp/RtpPacket_m.h"
#include "inet/transportlayer/rtp/RtpParticipantInfo.h"

namespace inet {
namespace rtp {

Register_Class(RtpParticipantInfo);

RtpParticipantInfo::RtpParticipantInfo(uint32 ssrc) :
    RtpParticipantInfo_Base(),
    _sdesChunk("SdesChunk", ssrc)
{
    setName(ssrcToName(ssrc).c_str());
    // because there haven't been sent any RTP packets
    // by this endsystem at all, the number of silent
    // intervals would be undefined; to calculate with
    // it but not to regard this endsystem as a sender
    // it is set to 3; see isSender() for details
    _silentIntervals = 3;
}

RtpParticipantInfo::RtpParticipantInfo(const RtpParticipantInfo& other) : RtpParticipantInfo_Base(other)
{
    copy(other);
}

RtpParticipantInfo::~RtpParticipantInfo()
{
}

RtpParticipantInfo& RtpParticipantInfo::operator=(const RtpParticipantInfo& other)
{
    if (this == &other)
        return *this;
    RtpParticipantInfo_Base::operator=(other);
    copy(other);
    return *this;
}

inline void RtpParticipantInfo::copy(const RtpParticipantInfo& other)
{
    _sdesChunk = other._sdesChunk;
    _silentIntervals = other._silentIntervals;
}

RtpParticipantInfo *RtpParticipantInfo::dup() const
{
    return new RtpParticipantInfo(*this);
}

void RtpParticipantInfo::processRTPPacket(Packet *packet, int id, simtime_t arrivalTime)
{
    packet->peekAtFront<RtpHeader>();
    _silentIntervals = 0;
    delete packet;
}

void RtpParticipantInfo::processSenderReport(const SenderReport& report, simtime_t arrivalTime)
{
    // useful code can be found in subclasses
}

void RtpParticipantInfo::processReceptionReport(const ReceptionReport& report, simtime_t arrivalTime)
{
    // useful code can be found in subclasses
}

void RtpParticipantInfo::processSDESChunk(const SdesChunk *sdesChunk, simtime_t arrivalTime)
{
    for (int i = 0; i < sdesChunk->size(); i++) {
        if (sdesChunk->exist(i)) {
            const SdesItem *sdesItem = check_and_cast<const SdesItem *>(sdesChunk->get(i));
            addSDESItem(sdesItem->dup());
        }
    }
}

SdesChunk *RtpParticipantInfo::getSDESChunk() const
{
    return new SdesChunk(_sdesChunk);
}

void RtpParticipantInfo::addSDESItem(SdesItem *sdesItem)
{
    _sdesChunk.addSDESItem(sdesItem);
}

bool RtpParticipantInfo::isSender() const
{
    return _silentIntervals <= 1;
}

ReceptionReport *RtpParticipantInfo::receptionReport(simtime_t now)
{
    return nullptr;
}

SenderReport *RtpParticipantInfo::senderReport(simtime_t now)
{
    return nullptr;
}

void RtpParticipantInfo::nextInterval(simtime_t now)
{
    _silentIntervals++;
}

bool RtpParticipantInfo::toBeDeleted(simtime_t now)
{
    return false;
}

uint32 RtpParticipantInfo::getSsrc() const
{
    return _sdesChunk.getSsrc();
}

void RtpParticipantInfo::setSsrc(uint32 ssrc)
{
    _sdesChunk.setSsrc(ssrc);
}

void RtpParticipantInfo::addSDESItem(SdesItem::SdesItemType type, const char *content)
{
    _sdesChunk.addSDESItem(new SdesItem(type, content));
}

std::string RtpParticipantInfo::ssrcToName(uint32 ssrc)
{
    char name[9];
    sprintf(name, "%08x", ssrc);
    return name;
}

} // namespace rtp
} // namespace inet

