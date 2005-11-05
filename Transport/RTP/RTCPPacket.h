/***************************************************************************
                          RTCPPacket.h  -  description
                             -------------------
    begin                : Sun Oct 21 2001
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

/** \file RTCPPacket.h
 * In this file all rtcp packet types are declared: There is a the superclass
 * RTCPPacket which is not intended to be used directly. It defines an enumeration
 * to distinguish the different rtcp packet types and also includes header
 * fields common to all types of rtcp packets.
 * Direct subclasses are RTCPReceiverReportPacket, RTCPSDESPacket and RTCPByePacket.
 * RTCPSenderReportPacket is declared as a subclass of RTCPReceiverReportPacket
 * because it only extends it with a sender report.
 * Application specific rtcp packets are not defined.
 * The class RTCPCompoundPacket isn't derived from RTCPPacket because it just
 * acts as a container for rtcp packets. Only rtcp compound packets are sent
 * over the network.
 */

#ifndef __RTCPPACKET_H__
#define __RTCPPACKET_H__

#include <iostream>
#include <omnetpp.h>
#include "INETDefs.h"
#include "types.h"
#include "reports.h"
#include "sdes.h"

/**
 * This is a base class for all types (except RTCPCompoundPacket) of rtcp
 * packets. It isn't intended to be used directly.
 */
class INET_API RTCPPacket : public cPacket
{

    public:

        /**
         * The values for the packet type field in the rtcp header as defined
         * in the rfc.
         */
        enum RTCP_PACKET_TYPE {
            RTCP_PT_UNDEF =   0, //!< default value undefined
            RTCP_PT_SR    = 200, //!< sender report
            RTCP_PT_RR    = 201, //!< receiver report
            RTCP_PT_SDES  = 202, //!< source description
            RTCP_PT_BYE   = 203  //!< bye
        };

        /**
         * Default constructor.
         */
        RTCPPacket(const char *name = NULL);

        /**
         * Copy constructor.
         */
        RTCPPacket(const RTCPPacket& rtcpPacket);

        /**
         * Destructor.
         */
        virtual ~RTCPPacket();

        /**
         * Assignment operator.
         */
        RTCPPacket& operator=(const RTCPPacket& rtcpPacket);

        /**
         * Return the class name "RTCPPacket".
         */
        virtual const char *className() const;

        /**
         * Duplicates the RTCPPacket by calling the copy constructor.
         */
        virtual cObject *dup() const;

        /**
         * Writes a short info about this RTCPPacket into the given buffer.
         */
        virtual std::string info();

        /**
         * Writes a detailed report about this RTCPPacket into the given stream.
         */
        virtual void writeContents(std::ostream& os) const;

        /**
         * Returns the rtp version of the rtcp packet. It's always 2.
         */
        virtual int version();

        /**
         * 1 if padding exists, 0 otherwise. In this implementation only
         * 0 is used.
         */
        virtual int padding();

        /**
         * Returns the value of the count field in the rtcp header. Depending
         * on the type of rtcp packet it stands for number of receiver reports
         * or number of sdes chunks contained in this packet.
         */
        virtual int count();

        /**
         * Returns the packet type of this rtcp packet.
         */
        virtual RTCP_PACKET_TYPE packetType();

        /**
         * Returns the value of the field length in the rtcp header.
         * The value isn't stored because it can be calculated
         * with the length() method inherited from cPacket.
         */
        virtual int rtcpLength() const;


    protected:

        /**
         * The rtp version used. Always 2.
         */
        int _version;

        /**
         * Set to 1 if padding (bytes at the end of the packet to assure
         * that the packet length in bytes is a multiple of a certain number;
         * possibly needed for encryption) is used. In the simulation padding
         */
        int _padding;

        /**
         * Depending on the packet type, here is stored how many receiver reports
         * or sdes chunks are contained in the packet. Values from 0 to 31
         * are allowed.
         */
        int _count;

        /**
         * The packet type of the rtcp packet.
         */
        RTCP_PACKET_TYPE _packetType;
};


/**
 * This class represents rtcp receiver report packets. It can hold 0 to 31
 * ReceptionReports. Also the header field ssrc is included.
 * \sa ReceptionReport
 */
class INET_API RTCPReceiverReportPacket : public RTCPPacket
{

    public:

        /**
         * Default contructor.
         */
        RTCPReceiverReportPacket(const char *name = NULL);

        /**
         * Copy constructor.
         */
        RTCPReceiverReportPacket(const RTCPReceiverReportPacket& rtcpReceiverReportPacket);

        /**
         * Destructor.
         */
        virtual ~RTCPReceiverReportPacket();

        /**
         * Assignment operator.
         */
        RTCPReceiverReportPacket& operator=(const RTCPReceiverReportPacket& rtcpReceiverReportPacket);

        /**
         * Duplicates the RTCPReceiverReportPacket by calling the copy constructor.
         */
        virtual cObject *dup() const;

        /**
         * Returns the class name "RTCPReceiverReportPacket".
         */
        virtual const char *className() const;

        /**
         * Reports a one line info about the RTCPReceiverReportPacket.
         */
        virtual std::string info();

        /**
         * Writes a report about the RTCPReceiverReportPacket into the stream.
         */
        virtual void writeContents(std::ostream& os) const;

        /**
         * Returns the ssrc indentifier of the source which has sent this
         * rtcp receiver report packet.
         */
        virtual u_int32 ssrc();

        /**
         * Sets the ssrc identifier for the rtcp receiver report packet.
         */
        virtual void setSSRC(u_int32 ssrc);

        /**
         * Adds a receiver report to this receiver report packet.
         */
        virtual void addReceptionReport(ReceptionReport *report);

        /**
         * Return a copy of the cArray of receiver reports stored
         * in the object.
         */
        virtual cArray *receptionReports();

    protected:

        /**
         * The ssrc identifier of the source of this rtcp packet.
         */
        u_int32 _ssrc;

        /**
         * The reception reports in this packet are stored here.
         */
        cArray *_receptionReports;

};


/**
 * This class represents rtcp sender report packets.  A rtcp sender report packet
 * is very similar to an rtcp receiver report packet with the only difference that
 * it includes exactly one sender report. To express this similarity it is a subclass
 * of RTPReceiverReportPacket.
 * \sa SenderReport
 * \sa ReceptionReport
 */
class INET_API RTCPSenderReportPacket : public RTCPReceiverReportPacket
{

    public:

        /**
         * Default constructor.
         */
        RTCPSenderReportPacket(const char *name = NULL);

        /**
         * Copy constructor.
         */
        RTCPSenderReportPacket(const RTCPSenderReportPacket& rtcpSenderReportPacket);

        /**
         * Destructor.
         */
        virtual ~RTCPSenderReportPacket();

        /**
         * Assignment operator.
         */
        RTCPSenderReportPacket& operator=(const RTCPSenderReportPacket& rtcpSenderReportPacket);

        /**
         * Duplicates the RTCPSenderReportPacket by calling the copy constructor.
         */
        virtual cObject *dup() const;

        /**
         * Return the class name "RTCPSenderReportPacket".
         */
        virtual const char *className() const;

        /**
         * Writes a one line info about this RTCPSenderReportPacket into the given string.
         */
        virtual std::string info();

        /**
         * Writes a longer info about this RTCPSenderReportPacket into the given stream.
         */
        virtual void writeContents(std::ostream& os) const;

        /**
        Returns a copy of the  sender report stored in this RTCPSenderReportPacket.
        \sa SenderReport
        */
        virtual SenderReport *senderReport();

        /**
        Sets the sender report.
        \sa SenderReport
        */
        virtual void setSenderReport(SenderReport *senderReport);

    private:

        /**
        The sender report stored in the packet.
        \sa SenderReport
        */
        SenderReport *_senderReport;
};


/**
 * An object of this class holds 0 to 31 source description chunks for
 * participants of the rtp session.
 * \sa SDESChunk
 */
class INET_API RTCPSDESPacket : public RTCPPacket
{

    public:
        /**
         * Default constructor.
         */
        RTCPSDESPacket(const char *name = NULL);

        /**
         * Copy constructor.
         */
        RTCPSDESPacket(const RTCPSDESPacket& rtcpSDESPacket);

        /**
         * Destructor.
         */
        virtual ~RTCPSDESPacket();

        /**
         * Assignment operator.
         */
        RTCPSDESPacket& operator=(const RTCPSDESPacket& rtcpSDESPacket);

        /**
         * Duplicates the RTCPSDESPacket by calling the copy constructor.
         */
        virtual cObject *dup() const;

        /**
         * Return the class name "RTCPSDESPacket".
         */
        virtual const char *className() const;

        /**
         * Writes a short info about this RTCPSDESPacket into the given string.
         */
        virtual std::string info();

        /**
         * Writes a longer report about this RTCPSDESPacket into the given stream.
         */
        virtual void writeContents(std::ostream& os) const;

        /**
         * Returns a copy of the cArray where the sdes chunks are stored.
         */
        virtual cArray *sdesChunks();

        /**
        Adds an sdes chunk to this rtcp sdes packet.
        \sa SDESChunk
        */
        virtual void addSDESChunk(SDESChunk *sdesChunk);

    private:

        /**
         * In this cArray the sdes chunks are stored.
         */
        cArray *_sdesChunks;
};


/**
 * An RTCPByePacket is used to indicate that an rtp endsystem
 * has left the session.
 * This implementation offers less functionality than described
 * in the rfc: Only one ssrc identifier can be stored in it and
 * the reason for leaving isn't transmitted.
 */
class INET_API RTCPByePacket : public RTCPPacket
{

    public:
        /**
         * Default constructor.
         */
        RTCPByePacket(const char *name = NULL);

        /**
         * Copy constructor.
         */
        RTCPByePacket(const RTCPByePacket& rtcpByePacket);

        /**
         * Destructor.
         */
        virtual ~RTCPByePacket();

        /**
         * Assignment operator.
         */
        RTCPByePacket& operator=(const RTCPByePacket& rtcpByePacket);

        /**
         * Duplicates the RTCPByePacket by calling the copy constructor.
         */
        virtual cObject *dup() const;

        /**
         * Returns the class name "RTCPByePacket".
         */
        virtual const char *className() const;

        /**
         * Returns the ssrc identifier.
         */
        virtual u_int32 ssrc();

        /**
         * Sets the ssrc identifier.
         */
        virtual void setSSRC(u_int32 ssrc);

    protected:
        /**
         * The ssrc identifier.
         */
        u_int32 _ssrc;
};


/**
 * An rtcp compound packet acts as container for rtcp packets, which
 * are transmitted in an RTCPCompoundPacket. Every RTCPCompoundPacket
 * must consist at least one RTCPSenderReportPacketof RTCPReceiverReportPacket and
 * one RTCPSDESPacket. This class doesn't check if these requirements are
 * met.
 */
class INET_API RTCPCompoundPacket : public cPacket
{

    public:
        /**
         * Default constructor.
         */
        RTCPCompoundPacket(const char *name = NULL);

        /**
         * Copy constructor.
         */
        RTCPCompoundPacket(const RTCPCompoundPacket& rtcpCompoundPacket);

        /**
         * Destructor.
         */
        virtual ~RTCPCompoundPacket();

        /**
         * Assignment operator.
         */
        RTCPCompoundPacket& operator=(const RTCPCompoundPacket& rtcpCompoundPacket);

        /**
         * Duplicates the RTCPCompoundPacket by calling the copy constructor.
         */
        virtual cObject *dup() const;

        /**
         * Return the class name "RTCPCompoundPacket".
         */
        virtual const char *className() const;

        /**
         * Writes a short info about this RTCPCompoundPacket into the given string.
         */
        virtual std::string info();

        /**
         * Writes a longer info about this RTCPCompoundPacket into the given stream.
         */
        virtual void writeContents(std::ostream& os) const;

        /**
         * Adds an RTCPPacket to this RTCPCompoundPacket.
         */
        virtual void addRTCPPacket(RTCPPacket *rtcpPacket);

        /**
         * Returns a copy of the cArray in which the rtcp
         * packets are stored.
         */
        virtual cArray *rtcpPackets();

    private:

        /**
         * The cArray in which the rtcp packets are stored.
         */
        cArray *_rtcpPackets;

};
#endif


