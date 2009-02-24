/***************************************************************************
                          RTPInnerPacket.h  -  description
                             -------------------
    begin                : Sat Oct 20 2001
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

/** \file RTPInnerPacket.h
 * This file declares the class RTPInnerPacket.
 */

#ifndef __INET_RTPINNERPACKET_H
#define __INET_RTPINNERPACKET_H

#include "INETDefs.h"
#include "IPAddress.h"
#include "RTPPacket.h"
#include "RTPSenderControlMessage_m.h"
#include "RTPSenderStatusMessage_m.h"


/**
 * This class is used for communication between submodules of the rtp layer module.
 */
class INET_API RTPInnerPacket : public cPacket
{

    public:

        /**
         * This enumeration is a list of all possibly types of
         * an RTPInnerPacket.
         */
        enum RTP_INP_TYPE {
            RTP_INP_UNDEF,
            RTP_INP_INITIALIZE_PROFILE,
            RTP_INP_PROFILE_INITIALIZED,
            RTP_INP_INITIALIZE_RTCP,
            RTP_INP_RTCP_INITIALIZED,
            RTP_INP_CREATE_SENDER_MODULE,
            RTP_INP_SENDER_MODULE_CREATED,
            RTP_INP_DELETE_SENDER_MODULE,
            RTP_INP_SENDER_MODULE_DELETED,
            RTP_INP_INITIALIZE_SENDER_MODULE,
            RTP_INP_SENDER_MODULE_INITIALIZED,
            RTP_INP_SENDER_MODULE_CONTROL,
            RTP_INP_SENDER_MODULE_STATUS,
            RTP_INP_LEAVE_SESSION,
            RTP_INP_SESSION_LEFT,
            RTP_INP_DATA_OUT,
            RTP_INP_DATA_IN
        };

        /**
         * Default constructor
         */
        RTPInnerPacket(const char *name = NULL);

        /**
         * Copy constructor.
         */
        RTPInnerPacket(const RTPInnerPacket& rinp);

        /**
         * Destructor.
         */
        virtual ~RTPInnerPacket();

        /**
         * Assignment operator.
         */
        RTPInnerPacket& operator=(const RTPInnerPacket& rinp);

        /**
         * Duplicates the RTPInnerPacket by calling the copy constructor.
         */
        virtual RTPInnerPacket *dup() const;

        /**
         * Writes a short info about this RTPInnerPacket into the given string.
         */
        virtual std::string info();

        /**
         * Writes a longer info about this RTPInnerPacket into the given output stream.
         */
        virtual void dump(std::ostream& os) const;

        /**
         * Called by the rtp module after creating the profile module. It
         * informes the profile about the maximum size an rtp packet can have.
         */
        virtual void initializeProfile(int mtu);

        /**
         * Called by the profile module after it has received the initializeProfile()
         * message. It informs the rtp module about the percentage of the available
         * bandwidth to be used by rtcp and the preferred port for this profile.
         */
        virtual void profileInitialized(int rtcpPercentage, int port);

        /**
         * Called by the rtp module to inform the rtcp module about mandatory
         * information for starting the rtp session.
         */
        virtual void initializeRTCP(const char *commonName, int mtu, int bandwidth, int rtcpPercentage, IPAddress address, int port);

        /**
         * Called by the rtcp module after it has waited for half an rtcp interval
         * for incoming messages from other session participants. It informs the rtp
         * module which later informs the rtp application about the ssrc identifier
         */
        virtual void rtcpInitialized(uint32 ssrc);

        virtual void createSenderModule(uint32 ssrc, int payloadType, const char *fileName);
        virtual void senderModuleCreated(uint32 ssrc);

        virtual void deleteSenderModule(uint32 ssrc);
        virtual void senderModuleDeleted(uint32 ssrc);

        virtual void initializeSenderModule(uint32 ssrc, const char *fileName, int mtu);
        virtual void senderModuleInitialized(uint32 ssrc, int payloadType, int clockRate, int timeStampBase, int sequenceNumberBase);

        virtual void senderModuleControl(uint32 ssrc, RTPSenderControlMessage *msg);
        virtual void senderModuleStatus(uint32 ssrc, RTPSenderStatusMessage *msg);

        /**
         * Called by the rtp module to inform the rtcp module that the session
         * should be left.
         */
        virtual void leaveSession();

        /**
         * Called by the rtcp module when the rtcp bye packet has been sent
         * to the network.
         */
        virtual void sessionLeft();

        /**
         * Capsulates the outgoing RTPPacket into this RTPInnerPacket to transport
         * it within the rtp layer.
         */
        virtual void dataOut(RTPPacket *packet);

        /**
         * Capsultes the incoming RTPPacket into this RTPInnerPacket to transport
         * it within the rtp layer.
         */
        virtual void dataIn(RTPPacket *packet, IPAddress address, int port);

        /**
         * Returns the type of this RTPInnerPacket.
         */
        virtual RTP_INP_TYPE getType();

        /**
         * Returns the CNAME stored in this RTPInnerPacket.
         */
        virtual const char *getCommonName();

        /**
         * Returns the maximum transmission unit stored in this RTPInnerPacket.
         */
        virtual int getMTU();

        /**
         * Returns the available bandwitdth as stored in this RTPInnerPacket.
         */
        virtual int getBandwidth();

        /**
         * Returns the percentage of bandwidth for rtcp as stored in this RTPInnerPacket.
         */
        virtual int getRtcpPercentage();

        /**
         * Returns the address stored in this RTPInnerPacket.
         */
        virtual IPAddress getAddress();

        /**
         * Returns the port stored in this RTPInnerPacket.
         */
        virtual int getPort();

        /**
         * Returns the ssrc identifier stored in this RTPInnerPacket.
         */
        virtual uint32 getSSRC();

        /**
         * Returns the payload type stored in this RTPInnerPacket.
         */
        virtual int getPayloadType();

        /**
         * Returns the file name stored in this RTPInnerPacket.
         */
        virtual const char *getFileName();

        /**
         * Returns the rtp clock rate stored in this RTPInnerPacket.
         */
        virtual int getClockRate();

        /**
         * Returns the rtp time stamp base stored in this RTPInnerPacket.
         */
        virtual int getTimeStampBase();

        /**
         * Returns the rtp sequence number base stored in this RTPInnerPacket.
         */
        virtual int getSequenceNumberBase();


    protected:

        /**
         * The type of this RTPInnerPacket.
         */
        RTP_INP_TYPE _type;

        /**
         * The CNAME stored in this RTPInnerPacket.
         */
        const char *_commonName;

        /**
         * The mtu stored in this RTPInnerPacket.
         */
        int _mtu;

        /**
         * The bandwidth stored in this RTPInnerPacket.
         */
        int _bandwidth;

        /**
         * The rtcp percentage stored in this RTPInnerPacket.
         */
        int _rtcpPercentage;

        /**
         * The address stored this RTPInnerPacket.
         */
        IPAddress _address;

        /**
         * The port stored this RTPInnerPacket.
         */
        int _port;

        /**
         * The ssrc identifier stored in this RTPInnerPacket.
         */
        uint32 _ssrc;

        /**
         * The payload type stored in this RTPInnerPacket.
         */
        int _payloadType;

        /**
         * The file name stored in this RTPInnerPacket.
         */
        const char *_fileName;

        /**
         * The clock rate stored in this RTPInnerPacket.
         */
        int _clockRate;

        /**
         * The rtp time stamp base stored in this RTPInnerPacket.
         */
        int _timeStampBase;

        /**
         * The rtp sequence number base stored in this RTPInnerPacket.
         */
        int _sequenceNumberBase;
};

#endif

