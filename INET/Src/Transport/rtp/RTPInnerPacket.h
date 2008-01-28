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

#ifndef __RTPINNERPACKET_H__
#define __RTPINNERPACKET_H__

#include <omnetpp.h>

//XXX #include "in_addr.h"
//XXX #include "in_port.h"
#include "tmp/defs.h"

#include "types.h"
#include "RTPPacket.h"
#include "RTPSenderControlMessage.h"
#include "RTPSenderStatusMessage.h"


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
        virtual cObject *dup() const;

        /**
         * Returns the class name "RTPInnerPacket".
         */
        virtual const char *className() const;

        /**
         * Writes a short info about this RTPInnerPacket into the given string.
         */
        virtual std::string info();

        /**
         * Writes a longer info about this RTPInnerPacket into the given output stream.
         */
        virtual void writeContents(std::ostream& os) const;

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
        virtual void profileInitialized(int rtcpPercentage, IN_Port port);

        /**
         * Called by the rtp module to inform the rtcp module about mandatory
         * information for starting the rtp session.
         */
        virtual void initializeRTCP(const char *commonName, int mtu, int bandwidth, int rtcpPercentage, IN_Addr address, IN_Port port);

        /**
         * Called by the rtcp module after it has waited for half an rtcp interval
         * for incoming messages from other session participants. It informs the rtp
         * module which later informs the rtp application about the ssrc identifier
         */
        virtual void rtcpInitialized(u_int32 ssrc);


        virtual void createSenderModule(u_int32 ssrc, int payloadType, const char *fileName);
        virtual void senderModuleCreated(u_int32 ssrc);

        virtual void deleteSenderModule(u_int32 ssrc);
        virtual void senderModuleDeleted(u_int32 ssrc);

        virtual void initializeSenderModule(u_int32 ssrc, const char *fileName, int mtu);
        virtual void senderModuleInitialized(u_int32 ssrc, int payloadType, int clockRate, int timeStampBase, int sequenceNumberBase);

        virtual void senderModuleControl(u_int32 ssrc, RTPSenderControlMessage *msg);
        virtual void senderModuleStatus(u_int32 ssrc, RTPSenderStatusMessage *msg);

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
        virtual void dataIn(RTPPacket *packet, IN_Addr address, IN_Port port);

        /**
         * Returns the type of this RTPInnerPacket.
         */
        virtual RTP_INP_TYPE type();

        /**
         * Returns the CNAME stored in this RTPInnerPacket.
         */
        virtual const char *commonName();

        /**
         * Returns the maximum transmission unit stored in this RTPInnerPacket.
         */
        virtual int mtu();

        /**
         * Returns the available bandwitdth as stored in this RTPInnerPacket.
         */
        virtual int bandwidth();

        /**
         * Returns the percentage of bandwidth for rtcp as stored in this RTPInnerPacket.
         */
        virtual int rtcpPercentage();

        /**
         * Returns the address stored in this RTPInnerPacket.
         */
        virtual IN_Addr address();

        /**
         * Returns the port stored in this RTPInnerPacket.
         */
        virtual IN_Port port();

        /**
         * Returns the ssrc identifier stored in this RTPInnerPacket.
         */
        virtual u_int32 ssrc();

        /**
         * Returns the payload type stored in this RTPInnerPacket.
         */
        virtual int payloadType();

        /**
         * Returns the file name stored in this RTPInnerPacket.
         */
        virtual const char *fileName();

        /**
         * Returns the rtp clock rate stored in this RTPInnerPacket.
         */
        virtual int clockRate();

        /**
         * Returns the rtp time stamp base stored in this RTPInnerPacket.
         */
        virtual int timeStampBase();

        /**
         * Returns the rtp sequence number base stored in this RTPInnerPacket.
         */
        virtual int sequenceNumberBase();


    private:

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
        IN_Addr _address;

        /**
         * The port stored this RTPInnerPacket.
         */
        IN_Port _port;

        /**
         * The ssrc identifier stored in this RTPInnerPacket.
         */
        u_int32 _ssrc;

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

