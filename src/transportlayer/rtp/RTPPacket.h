/***************************************************************************
                          RTPPacket.h  -  description
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


#include "RTPPacket_m.h"

/**
 * This class represents an RTP data packet.
 * Real data can either be encapsulated or simulated by
 * adding length.
 * Following RTP header fields exist but aren't used: padding, extension,
 * csrcCount. The csrcList can't be used because csrcCount is always 0.
 */
class RTPPacket : public RTPPacket_Base
{
  public:
    RTPPacket(const char *name = NULL, int kind = 0) : RTPPacket_Base(name, kind) {}
    RTPPacket(const RTPPacket& other) : RTPPacket_Base(other) {}
    RTPPacket& operator=(const RTPPacket& other) {RTPPacket_Base::operator=(other); return *this;}
    virtual RTPPacket *dup() const {return new RTPPacket(*this);}
    // ADD CODE HERE to redefine and implement pure virtual functions from RTPPacket_Base

    /**
     * Writes a one line info about this RTPPacket into the given string.
     */
    virtual std::string info() const;

    /**
     * Writes a longer description about this RTPPacket into the given stream.
     */
    virtual void dump() const;

    /**
     * Returns the length of the header (fixed plus variable part)
     * of this RTPPacket.
     */
    virtual int getHeaderLength() const;
    virtual void setHeaderLength(int x) { throw cRuntimeError("Don't use SetHeaderLength()"); }

    /**
     * Returns the size of the payload stored in this RTPPacket.
     */
    virtual int getPayloadLength() const;
    virtual void setPayloadLength(int x) { throw cRuntimeError("Don't use SetPayloadLength()"); }
};

#endif
