/***************************************************************************
                          RtpHeader.h  -  description
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

#ifndef __INET_RTPPACKET_H
#define __INET_RTPPACKET_H

#include "inet/transportlayer/rtp/RTPPacket_m.h"

namespace inet {

namespace rtp {

/**
 * This class represents an RTP data packet.
 * Real data can either be encapsulated or simulated by
 * adding length.
 * Following RTP header fields exist but aren't used: padding, extension,
 * csrcCount. The csrcList can't be used because csrcCount is always 0.
 */
class INET_API RtpHeader : public RtpHeader_Base
{
  public:
    RtpHeader() : RtpHeader_Base() {}
    RtpHeader(const RtpHeader& other) : RtpHeader_Base(other) {}
    RtpHeader& operator=(const RtpHeader& other) { RtpHeader_Base::operator=(other); return *this; }
    virtual RtpHeader *dup() const override { return new RtpHeader(*this); }
    // ADD CODE HERE to redefine and implement pure virtual functions from RTPPacket_Base

    /**
     * Writes a one line info about this RtpHeader into the given string.
     */
    virtual std::string info() const override;

    /**
     * Writes a longer description about this RtpHeader into the given stream.
     */
    virtual void dump() const;
};

} // namespace rtp

} // namespace inet

#endif // ifndef __INET_RTPPACKET_H

