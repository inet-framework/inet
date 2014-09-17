/***************************************************************************
                          RTPAVProfilePayload32Sender.h  -  description
                             -------------------
    begin                : Wed Nov 28 2001
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

#ifndef __INET_RTPAVPROFILEPAYLOAD32SENDER_H
#define __INET_RTPAVPROFILEPAYLOAD32SENDER_H

#include "inet/common/INETDefs.h"

#include "inet/transportlayer/rtp/RTPPayloadSender.h"

namespace inet {

namespace rtp {

/**
 * An RTPAVProfilePayload32Sender is a module for sending data of payload
 * type 32 in the rtp audio/video profile, which is mpeg video.
 * This implementation doesn't send real mpeg data it just reads the gdf
 * file created by Mpeg_Stat and sends rtp data packets which contain
 * an RTPMpegPacket.
 * The corresponding receiver module RTPAVProfilePayload32Receiver.
 */
class INET_API RTPAVProfilePayload32Sender : public RTPPayloadSender
{
  protected:
    /**
     * Initializes the module. It sets the values for clock rate and payload type.
     */
    virtual void initialize();

    /**
     * The main method.
     */
    //virtual void activity();

  protected:
    /**
     * This method reads the gdf file header.
     */
    virtual void initializeSenderModule(RTPInnerPacket *rinpIn);

    /**
     * This method sends one mpeg frame. It sends one or more
     * rtp data packet. Returns false if there were no more frames.
     */
    virtual bool sendPacket();

    /**
     * The initial delay of the mpeg video.
     */
    double _initialDelay;

    /**
     * The number of frames per second of the mpeg video.
     */
    double _framesPerSecond;

    /**
     * The number of the current mpeg frame. Needed for calculating
     * the rtp time stamp in the rtp data packets.
     */
    double _frameNumber;
};

} // namespace rtp

} // namespace inet

#endif    // __INET_RTPAVPROFILEPAYLOAD32SENDER_H

