/***************************************************************************
                          reports.h  -  description
                             -------------------
    begin                : Tue Oct 23 2001
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

/** \file reports.h
 * This file declares the classes SenderReport and ReceptionReport as used
 * in RTCPSenderReportPacket and RTCPReceiverReportPacket.
 */

#ifndef __REPORTS_H__
#define __REPORTS_H__

#include <omnetpp.h>

#include "INETDefs.h"
#include "types.h"
#include "IPAddress.h"


/**
 * The class SenderReport represents an rtp sender report as contained
 * in an RTCPSenderReportPacket.
 */
class INET_API SenderReport : public cObject
{

    public:

        /**
         * Default constructor.
         */
        SenderReport();

        /**
         * Copy constructor. Needed by omnet++.
         */
        SenderReport(const SenderReport& senderReport);

        /**
         * Destructor.
         */
        virtual ~SenderReport();

        /**
         * Assignment operator.
         */
        SenderReport& operator=(const SenderReport& senderReport);

        /**
         * Duplicates this SenderReport by calling the copy constructor.
         */
        virtual SenderReport *dup() const;

        /**
         * Writes a short info about this SenderReport into the given string.
         */
        virtual std::string info();

        /**
         * Writes a longer info about this SenderReport into the given stream.
         */
        virtual void dump(std::ostream& os) const;

        /**
         * Returns the contained ntp time stamp.
         */
        virtual uint64_t getNTPTimeStamp();

        /**
         * Sets the ntp time stamp.
         */
        virtual void setNTPTimeStamp(uint64_t ntpTimeStamp);

        /**
         * Returns the contained rtp time stamp.
         */
        virtual uint32_t getRTPTimeStamp();

        /**
         * Sets the rtp time stamp.
         */
        virtual void setRTPTimeStamp(uint32_t timeStamp);

        /**
         * Returns the number of packets sent as stored in this SenderReport.
         */
        virtual uint32_t getPacketCount();

        /**
         * Sets the number of packets sent.
         */
        virtual void setPacketCount(uint32_t packetCount);

        /**
         * Returns how many bytes have been sent as store in this SenderReport.
         */
        virtual uint32_t getByteCount();

        /**
         * Sets the value how many bytes have been sent.
         */
        virtual void setByteCount(uint32_t byteCount);

    protected:

        /**
         * The ntp time stamp.
         */
        uint64_t _ntpTimeStamp;

        /**
         * The rtp time stamp.
         */
        uint32_t _rtpTimeStamp;

        /**
         * The number of packets sent.
         */
        uint32_t _packetCount;

        /**
         * The number of (payload) bytes sent.
         */
        uint32_t _byteCount;
};


/**
 * The class ReceptionReport represents an rtp receiver report stored
 * in an RTPSenderReportPacket or RTPReceiverReport.
 */
class INET_API ReceptionReport : public cObject
{

    public:
        /**
         * Default constructor.
         */
        ReceptionReport();

        /**
         * Copy constructor..
         */
        ReceptionReport(const ReceptionReport& receptionReport);

        /**
         * Destructor.
         */
        virtual ~ReceptionReport();

        /**
         * Assignment operator.
         */
        ReceptionReport& operator=(const ReceptionReport& receptionReport);

        /**
         * Duplicates this ReceptionReport by calling the copy constructor.
         */
        virtual ReceptionReport *dup() const;

        /**
         * Writes a short info about this ReceptionReport into the given string.
         */
        virtual std::string info();

        /**
         * Writes a longer info about this ReceptionReport into the given stream.
         */
        virtual void dump(std::ostream& os) const;

        /**
         * Returns the ssrc identifier for which sender this ReceptionReport is.
         */
        virtual uint32_t getSSRC();

        /**
         * Sets the ssrc identifier of the sender this ReceptionReport is for.
         */
        virtual void setSSRC(uint32_t ssrc);

        /**
         * Returns the fraction of packets lost as stored in this ReceptionReport.
         */
        virtual uint8_t getFractionLost();

        /**
         * Sets the fraction of packets lost.
         */
        virtual void setFractionLost(uint8_t fractionLost);

        /**
         * Returns the number of expected minus the number of packets received.
         */
        virtual int getPacketsLostCumulative();

        /**
         * Sets the number of expected minus the number of packets received.
         */
        virtual void setPacketsLostCumulative(int packetLostCumulative);

        /**
         * Returns the extended highest sequence number received.
         */
        virtual uint32_t getSequenceNumber();

        /**
         * Set the extended highest sequence number received.
         */
        virtual void setSequenceNumber(uint32_t sequenceNumber);

        /**
         * Returns the interarrival jitter.
         */
        virtual int getJitter();

        /**
         * Sets ths interarrival jitter.
         */
        virtual void setJitter(int jitter);

        /**
         * Returns the rtp time stamp of the last SenderReport received from this sender.
         */
        virtual int getLastSR();

        /**
         * Sets the rtp time stamp of the last SenderReport received from this sender.
         */
        virtual void setLastSR(int lastSR);

        /**
         * Returns the delay since the last SenderReport of this sender has
         * been received in units of 1/65536 seconds.
         */
        virtual int getDelaySinceLastSR();

        /**
         * Sets the delay since the last SenderReport of this sender has
         * been received in units of 1/65536 seconds.
         */
        virtual void setDelaySinceLastSR(int delaySinceLastSR);

    protected:

        /**
         * The ssrc identifier of the sender this ReceptionReport is for.
         */
        uint32_t _ssrc;

        /**
         * The fraction lost.
         */
        uint8_t _fractionLost;

        /**
         * The number of packets expected minus the number of packets received.
         */
        int _packetsLostCumulative;

        /**
         * The extended highest sequence number received.
         */
        uint32_t _extendedHighestSequenceNumber;

        /**
         * The interarrival jitter.
         */
        int _jitter;

        /**
         * The rtp time stamp of the last SenderReport received from this source.
         */
        int _lastSR;

        /**
         * The delay since the last SenderReport from this sender has been
         * received in units of 1/65536 seconds.
         */
        int _delaySinceLastSR;

};

#endif

