/***************************************************************************
                          RtpAvProfilePayload32Receiver.h  -  description
                             -------------------
    begin                : Sun Jan 6 2002
    copyright            : (C) 2002 by Matthias Oppitz
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

/** \file RtpAvProfilePayload32Receiver.h
 * This file declares the class RtpAvProfilePayload32Receiver.
 */

#ifndef __INET_RTPAVPROFILEPAYLOAD32RECEIVER_H
#define __INET_RTPAVPROFILEPAYLOAD32RECEIVER_H

#include "inet/common/INETDefs.h"

#include "inet/transportlayer/rtp/RtpPayloadReceiver.h"

namespace inet {

namespace rtp {

/**
 * This module is used to receive getData(mpeg video) of payload 32 for rtp
 * endsystems working under the rtp av profile.
 * It expects data in the format defined in rfc 2250.
 * Its corresponding sender module is RtpAvProfilePayload32Sender.
 * This implementation doesn't work with real mpeg data, so it doesn't write
 * an mpeg file but a sim file, which can be played with a modified
 * mpeg player.
 */
class INET_API RtpAvProfilePayload32Receiver : public RtpPayloadReceiver
{
  protected:
    /**
     * Destructor.
     */
    virtual ~RtpAvProfilePayload32Receiver();

    /**
     * Calls the method of the superclass RtpPayloadReceiver and sets the
     * payload type to 32.
     */
    virtual void initialize() override;

  protected:
    /**
     * A reordering queue for incoming packets.
     */
    cQueue *_queue;

    /**
     * Stores the lowest allowed time stamp of rtp data packets. The value
     * is used to throw away packets from mpeg frames already stored in
     * the data file.
     */
    uint32 _lowestAllowedTimeStamp;

    uint32 _highestSequenceNumber;

    /**
     * Writes information about received frames into the output file.
     * The only error correction provided is reordering packets
     * of one frame if needed. The packet should begin with RtpHeader.
     */
    virtual void processRtpPacket(Packet *packet) override;
};

} // namespace rtp

} // namespace inet

#endif // ifndef __INET_RTPAVPROFILEPAYLOAD32RECEIVER_H

