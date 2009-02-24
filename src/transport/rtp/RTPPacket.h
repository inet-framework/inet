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

/** \file RTPPacket.h
 * This file declares the class RTPPacket.
 */

#ifndef __INET_RTPPACKET_H
#define __INET_RTPPACKET_H

#include "INETDefs.h"

/**
 * This class represents an rtp data packet.
 * Real data can either be encapsulated or simulated by
 * adding length.
 * Following rtp header fields exist but aren't used: padding, extension,
 * csrcCount. The csrcList can't be used because csrcCount is always 0.
 */
class INET_API RTPPacket : public cPacket
{

    public:
        /**
         * Default constructor.
         */
        RTPPacket(const char *name = NULL);

        /**
         * Copy constructor.
         */
        RTPPacket(const RTPPacket& packet);

        /**
         * Destructor.
         */
        virtual ~RTPPacket();

        /**
         * Assignment operator.
         */
        RTPPacket& operator=(const RTPPacket& packet);

        /**
         * Duplicates the RTPPacket by calling the copy constructor.
         */
        virtual RTPPacket *dup() const;

        /**
         * Writes a one line info about this RTPPacket into the given string.
         */
        virtual std::string info();

        /**
         * Writes a longer description about this RTPPacket into the given stream.
         */
        virtual void dump();

        /**
         * Returns the value of the marker bit in this RTPPacket.
         */
        virtual int getMarker();

        /**
         * Sets the value of the marker bit in this RTPPacket.
         */
        virtual void setMarker(int marker);

        /**
         * Returns the payload type of this RTPPacket.
         */
        virtual int getPayloadType();

        /**
         * Sets the payload type of this RTPPacket.
         */
        virtual void setPayloadType(int payloadType);

        /**
         * Returns the sequence number of this RTPPacket.
         */
        virtual uint16 getSequenceNumber();

        /**
         * Sets the sequence number of this RTPPacket.
         */
        virtual void setSequenceNumber(uint16 sequenceNumber);

        /**
         * Returns the rtp time stamp of this RTPPacket.
         */
        virtual uint32 getTimeStamp();

        /**
         * Sets the rtp time stamp of this RTPPacket.
         */
        virtual void setTimeStamp(uint32 timeStamp);

        /**
         * Returns the ssrc identifier of this RTPPacket.
         */
        virtual uint32 getSSRC();

        /**
         * Sets the ssrc identifier of this RTPPacket.
         */
        virtual void setSSRC(uint32 ssrc);

        /**
         * Returns the length of the fixed header of an RTPPacket.
         */
        static int getFixedHeaderLength();

        /**
         * Returns the length of the header (fixed plus variable part)
         * of this RTPPacket.
         */
        virtual int getHeaderLength();

        /**
         * Returns the size of the payload stored in this RTPPacket.
         */
        virtual int getPayloadLength();

    protected:

        /**
         * The rtp version of this RTPPacket.
         */
        int _version;

        /**
         * Set to 1 if padding is used in this RTPPacket, 0 otherwise.
         * This implementation doesn't use padding bytes, so it is always 0.
         */
        int _padding;

        /**
         * Set to 1, if this RTPPacket contains an rtp header extension, 0 otherwise.
         * This implementation doesn't support rtp header extensions, so it is always 0.
         */
        int _extension;

        /**
         * Stores the number (0..31) of contributing sources for this RTPPacket.
         * It is always 0 because contributing sources are added by rtp mixers
         * which aren't implemented.
         */
        int _csrcCount;

        /**
         * The marker of this RTPPacket.
         */
        int _marker;

        /**
         * The type of payload carried in this RTPPacket.
         */
        int _payloadType;

        /**
         * The sequence number of this RTPPacket.
         */
        uint16 _sequenceNumber;

        /**
         * The rtp time stamp of this RTPPacket.
         */
        uint32 _timeStamp;

        /**
         * The ssrc identifier of the creator of this RTPPacket.
         */
        uint32 _ssrc;

        // no mixers, no contributing sources
        //int _csrc[];



};

#endif

