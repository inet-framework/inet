/***************************************************************************
                          RTPSenderInfo.h  -  description
                             -------------------
    begin                : Wed Dec 5 2001
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

#ifndef __INET_RTPSENDERINFO_H
#define __INET_RTPSENDERINFO_H

#include "inet/transportlayer/rtp/RTPParticipantInfo.h"

#include "inet/common/INETDefs.h"

namespace inet {

namespace rtp {

/**
 * The class RTPSenderInfo is used by an RTP end system for storing information
 * about itself. With the stored information it can create a SenderReport.
 */
class INET_API RTPSenderInfo : public RTPParticipantInfo
{
  public:
    /**
     * Default constructor.
     */
    RTPSenderInfo(uint32 ssrc = 0);

    /**
     * Copy constructor.
     */
    RTPSenderInfo(const RTPSenderInfo& senderInfo);

    /**
     * Destructor.
     */
    virtual ~RTPSenderInfo();

    /**
     * Assignment operator.
     */
    RTPSenderInfo& operator=(const RTPSenderInfo& senderInfo);

    /**
     * Duplicates this RTPSenderInfo by calling the copy constructor.
     */
    virtual RTPSenderInfo *dup() const;

    /**
     * Stores information about this outgoing RTPPacket.
     */
    virtual void processRTPPacket(RTPPacket *packet, int id, simtime_t arrivalTime);

    /**
     * Processes an incoming ReceptionReport for this sender.
     */
    virtual void processReceptionReport(ReceptionReport *report, simtime_t arrivalTime);

    /**
     * Returns a SenderReport for this rtp endsystem.
     * If it hasn't sent rtp data packets during the
     * last 2 rtcp intervals, it returns NULL.
     */
    virtual SenderReport *senderReport(simtime_t now);

    /**
     * Sets the time (simTime) when this endsystem has
     * started sending rtp packets.
     */
    virtual void setStartTime(simtime_t startTime);

    /**
     * Sets the clock rate (in ticks per second) this sender
     * increases the rtp time stamp.
     */
    virtual void setClockRate(int clockRate);

    /**
     * Sets the initial rtp time stamp.
     */
    virtual void setTimeStampBase(uint32 timeStampBase);

    /**
     * Sets the initial sequence number.
     */
    virtual void setSequenceNumberBase(uint16 sequenceNumberBase);

    /**
     * A sender info shall never be deleted!
     */
    virtual bool toBeDeleted(simtime_t now) const;

  private:
    void copy(const RTPSenderInfo& other);

  protected:
    /**
     * The time when the transmission was started.
     */
    simtime_t _startTime;

    /**
     * The clock rate this sender increases the rtp time stamp.
     */
    int _clockRate;

    /**
     * The initial rtp time stamp.
     */
    uint32 _timeStampBase;

    /**
     * The initial sequence number.
     */
    uint16 _sequenceNumberBase;

    /**
     * The number of rtp data packets this sender has sent.
     */
    uint32 _packetsSent;

    /**
     * The number of data bytes this sender has sent.
     */
    uint32 _bytesSent;
};

} // namespace rtp

} // namespace inet

#endif // ifndef __INET_RTPSENDERINFO_H

