/***************************************************************************
                          RTPReceiverInfo.h  -  description
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


/** \file RTPReceiverInfo.h
 * This file declares the class RTPReceiverInfo.
 */

#ifndef __RTPRECEIVERINFO_H__
#define __RTPRECEIVERINFO_H__

#include <omnetpp.h>

#include "types.h"
#include "RTPParticipantInfo.h"

/**
 * This class, a subclass of RTPParticipantInfo, is used for storing information
 * about other rtp endsystems.
 * This class processes rtp packets, rtcp sender reports and rtcp sdes chunks
 * originating from this endsystem.
 */
class INET_API RTPReceiverInfo : public RTPParticipantInfo
{

    public:

        /**
         * Default constructor.
         */
        RTPReceiverInfo(u_int32 ssrc = 0);

        /**
         * Copy constructor.
         */
        RTPReceiverInfo(const RTPReceiverInfo& receiverInfo);

        /**
         * Destructor.
         */
        virtual ~RTPReceiverInfo();

        /**
         * Assignment operator.
         */
        RTPReceiverInfo& operator=(const RTPReceiverInfo& receiverInfo);

        /**
         * Duplicates this RTPReceiverInfo by calling the copy constructor.
         */
        virtual cObject *dup() const;

        /**
         * Returns the class name "RTPReceiverInfo".
         */
        virtual const char *className() const;

        /**
         * Extracts information of the given RTPPacket.
         * Also sets _inactiveIntervals to 0.
         */
        virtual void processRTPPacket(RTPPacket *packet, simtime_t arrivalTime);

        /**
         * Extracts information of the given SenderReport.
         */
        virtual void processSenderReport(SenderReport *report, simtime_t arrivalTime);

        /**
         * Extracts information of the given SDESChunk.
         */
        virtual void processSDESChunk(SDESChunk *sdesChunk, simtime_t arrivalTime);

        /**
         * Returns a ReceptionReport if this rtp end system is a sender,
         * NULL otherwise.
         */
        virtual ReceptionReport *receptionReport(simtime_t now);

        /**
         * Informs this RTPReceiverInfo that one rtcp interval has past.
         */
        virtual void nextInterval(simtime_t now);

        /**
         * Returns true if this rtp end system is regarded active.
         */
        virtual bool active();

        /**
         * Returns true if this rtp end system is regarded valid.
         */
        virtual bool valid();

        /**
         * Returns true if this rtp end system should be deleted from
         * the list of known rtp session participant.
         * This method should be called directly after nextInterval().
         */
        virtual bool toBeDeleted(simtime_t now);


    private:

        /**
         * The sequence number of the first RTPPacket received.
         */
        u_int16 _sequenceNumberBase;

        /**
         * The highest sequence number of an RTPPacket received.
         */
        u_int16 _highestSequenceNumber;

        /**
         * The highest sequence number of an RTPPacket received
         * before the beginning of the current rtcp interval.
         */
        u_int32 _highestSequenceNumberPrior;

        /**
         * The number of sequence number wrap arounds.
         */
        u_int32 _sequenceNumberCycles;

        /**
         * How many rtp packets from this source have been received.
         */
        u_int32 _packetsReceived;

        /**
         * How many rtp packets have been received from this source
         * before the current rtcp interval began.
         */
        u_int32 _packetsReceivedPrior;

        /**
         * The interarrival jitter. See rtp rfc for details.
         */
        double _jitter;

        /**
         * The clock rate (in ticks per second) the sender increases the
         * rtp timestamps. It is calculated when two sender reports have
         * been received.
        */
        int _clockRate;

        /**
         * The rtp time stamp of the last SenderReport received from this sender.
         */
        u_int32 _lastSenderReportRTPTimeStamp;

        /**
         * The ntp time stamp of the last SenderReport received from this sender.
         */
        u_int64 _lastSenderReportNTPTimeStamp;

        /**
         * The rtp time stamp of the last RTPPacket received from this sender.
         * Needed for calculating the jitter.
         */
        u_int32 _lastPacketRTPTimeStamp;

        /**
         * The arrival time of the last RTPPacket received from this sender.
         * Needed for calculating the jitter.
         */
        simtime_t _lastPacketArrivalTime;

        /**
         * The arrival time of the last SenderReport received from this sender.
         */
        simtime_t _lastSenderReportArrivalTime;

        /**
         * The consecutive number of rtcp intervals this rtcp end system
         * hasn't sent anything.
         */
        int _inactiveIntervals;

        /**
         * The time when this rtp end system has been inactive for five
         * consecutive rtcp intervals.
         */
        simtime_t _startOfInactivity;

        /**
         * The number of rtp and rtcp packets received from this rtp end system.
         */
        int _itemsReceived;

};

#endif

