/***************************************************************************
                          RTPParticipantInfo.h  -  description
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

/** \file RTPParticipantInfo.h
 * TThis file declares the class RTPParticipantInfo.
 */

#ifndef __INET_RTPPARTICIPANTINFO_H
#define __INET_RTPPARTICIPANTINFO_H

#include <stdio.h>
#include "INETDefs.h"
#include "IPAddress.h"
#include "RTPPacket.h"
#include "RTCPPacket.h"
#include "reports.h"
#include "sdes.h"

/**
 * This class is a super class for classes intended for storing information
 * about rtp end systems.
 * It has two subclasses: RTPReceiverInformation which is used for storing
 * information about other system participating in an rtp session.
 * RTPSenderInformation is used by an rtp endsystem for storing information
 * about itself.
 * \sa RTPReceiverInformation
 * \sa RTPSenderInformation
 */
class INET_API RTPParticipantInfo : public cObject
{

    public:

        /**
         * Default constructor.
         */
        RTPParticipantInfo(uint32 ssrc = 0);

        /**
         * Copy constructor.
         */
        RTPParticipantInfo(const RTPParticipantInfo& participantInfo);

        /**
         * Destructor.
         */
        virtual ~RTPParticipantInfo();

        /**
         * Assignment operator.
         */
        RTPParticipantInfo& operator=(const RTPParticipantInfo& participantInfo);

        /**
         * Duplicates this RTPParticipantInfo by calling the copy constructor.
         */
        virtual RTPParticipantInfo *dup() const;

        /**
         * This method should be extended by a subclass for
         * extracting information about the originating
         * endsystem of an rtp packet.
         * This method sets _silentInterval to 0 so that
         * the sender of this rtp packet is regarded as
         * an active sender.
         */
        virtual void processRTPPacket(RTPPacket *packet, int id, simtime_t arrivalTime);

        /**
         * This method extracts information about an rtp endsystem
         * as provided by the given SenderReport.
         */
        virtual void processSenderReport(SenderReport *report, simtime_t arrivalTime);

        /**
         * This method extracts information of the given ReceptionReport.
         */
        virtual void processReceptionReport(ReceptionReport *report, simtime_t arrivalTime);

        /**
         * This method extracts sdes information of the given sdes chunk.and stores it.
         */
        virtual void processSDESChunk(SDESChunk *sdesChunk, simtime_t arrivalTime);

        /**
         * Returns a copy of the sdes chunk used for storing source
         * description items about this system.
         */
        virtual SDESChunk *getSDESChunk();

        /**
         * Adds this sdes item to the sdes chunk of this participant.
         */
        virtual void addSDESItem(SDESItem *sdesItem);

        /**
         * This method is intended to be overwritten by subclasses. It
         * should return a receiver report if there have been received
         * rtp packets from that endsystem and NULL otherwise.
         */
        virtual ReceptionReport *receptionReport(simtime_t now);

        /**
         * This method is intended to be overwritten by subclasses which
         * are used for storing information about itself.
         * It should return a sender report if there have been sent rtp
         * packets recently or NULL otherwise.
         * The implementation for this class always returns NULL.
         */
        virtual SenderReport *senderReport(simtime_t now);

        /**
         * This method should be called by the rtcp module which uses this class
         * for storing information every time an rtcp packet is sent.
         * Some behaviour of rtp and rtcp (and this class) depend on how
         * many rtcp intervals have passed, for example an rtp end system
         * is marked as inactive if there haven't been received packets from
         * it for a certain number of rtpc intervals.
         * Call getSenderReport() and createReceptionReport() before calling this method.
         * \sa getSenderReport()
         * \sa createReceptionReport()
         */
        virtual void nextInterval(simtime_t now);

        /**
         * Returns true if the end system does no longer participate
         * in the rtp session.
         * The implementation in this class always returns false.
         */
        virtual bool toBeDeleted(simtime_t now);

        /**
         * Returns true if this endsystem has sent at least one rtp
         * data packet during the last two rtcp intervals (including
         * the current one).
         */
        virtual bool isSender();

        /**
         * Returns the ssrc identifier of the rtp endsystem.
         */
        virtual uint32 getSSRC();

        /**
         * Sets the ssrc identifier.
         */
        virtual void setSSRC(uint32 ssrc);

        /**
         * Returns the ip address of the rtp endsystem.
         */
        virtual IPAddress getAddress();

        /**
         * Sets the ip address of the rtp endsystem.
         */
        virtual void setAddress(IPAddress address);

        /**
         * Returns the port used by this endsystem for
         * transmitting rtp packets.
         */
        virtual int getRTPPort();

        /**
         * Sets the port used by the endsystem for
         * transmitting rtp packets.
         */
        virtual void setRTPPort(int rtpPort);

        /**
         * Returns the port used by this endsystem for
         * transmitting rtcp packets.
         */
        virtual int getRTCPPort();

        /**
         * Sets the port used by the endsystem for
         * transmitting rtcp packets.
         */
        virtual void setRTCPPort(int rtpPort);

        /**
         * This method returns the given 32 bit ssrc identifier as
         * an 8 character hexadecimal number which is used as name
         * of an RTPParticipantInfo object.
         */
        static char *ssrcToName(uint32 ssrc);

        virtual void dump() const;

    protected:

        /**
         * Used for storing sdes information about this rtp endsystem.
         * The ssrc identifier is also stored here.
         */
        SDESChunk *_sdesChunk;

        /**
         * Used for storing the ip address of this endsystem.
         */
        IPAddress _address;

        /**
         * Used for storing the port for rtp by this endsystem.
         */
        int _rtpPort;

        /**
         * Used for storing the port for rtcp by this endsystem.
         */
        int _rtcpPort;

        /**
         * Stores the number of rtcp intervals (including the current one)
         * during which this rtp endsystem hasn't sent any rtp data packets.
         * When an rtp data packet is received it is reset to 0.
         */
        int _silentIntervals;

        /**
         * Creates a new SDESItem and adds it to the SDESChunk stored in
         * this RTPParticipantInfo.
         */
        virtual void addSDESItem(SDESItem::SDES_ITEM_TYPE type, const char *content);
};

#endif

