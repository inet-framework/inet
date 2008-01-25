/***************************************************************************
                          RTPMpegPacket.h  -  description
                             -------------------
    begin                : Fri Dec 14 2001
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

/** \file RTPMpegPacket.h
 * This file contains the declaration of the class RTPMpegPacket.
 */

#ifndef __RTPMPEGPACKET_H__
#define __RTPMPEGPACKET_H__

#include <omnetpp.h>
#include "INETDefs.h"

/**
 * An RTPMpegPacket is intended to be capsulated into an RTPPacket when
 * transmitting mpeg data with rtp under the rtp audio/video profile.
 * It stores information about the mpeg data as described in rfc 2250.
 * This implementation doesn't transport real mpeg data. It is intended
 * to simulate storing mpeg data by adding length.
 * Currently only one header field (picture type) is filled with the right
 * value. The values for the other header fields can't be determined by
 * reading the gdf file.
 * \sa RTPAVProfilePayload32Sender
 * \sa RTPAVProfilePayload32Receiver
 */
class INET_API RTPMpegPacket : public cPacket
{

    public:
        /**
         * Default constructor.
         */
        RTPMpegPacket(const char *name = NULL);

        /**
         * Copy constructor.
         */
        RTPMpegPacket(const RTPMpegPacket& packet);

        /**
         * Destructor.
         */
        virtual ~RTPMpegPacket();

        /**
         * Assignment operator.
         */
        RTPMpegPacket& operator=(const RTPMpegPacket& packet);

        /**
         * Duplicates the RTPMpegPacket by calling the copy constructor.
         */
        virtual cObject *dup() const;

        /**
         * Returns the class name "RTPMpegPacket".
         */
        virtual const char *className() const;

        /**
         * Returns the constant header length (4 bytes).
         */
        static int headerLength();

        /**
         * Returns the size of mpeg data.
         */
        virtual int payloadLength();

        /**
         * Returns the picture type of the frame the data in this
         * packet belongs to.
         */
        virtual int pictureType();

        /**
         * Sets the picture type.
         */
        virtual void setPictureType(int pictureType);

    private:

        //! Not used.
        int _mzb;

        //! Not used.
        int _two;

        //! Not used.
        int _temporalReference;

        //! Not used.
        int _activeN;

        //! Not used.
        int _newPictureHeader;

        //! Not used.
        int _sequenceHeaderPresent;

        //! Not used.
        int _beginningOfSlice;

        //! Not used.
        int _endOfSlice;

        //! The picture type of the frame this packet belongs to.
        int _pictureType;

        //! Not used.
        int _fbv;

        //! Not used.
        int _bfc;

        //! Not used.
        int _ffv;

        //! Not used.
        int _ffc;

};

#endif