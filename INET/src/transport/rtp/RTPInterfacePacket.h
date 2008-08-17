/***************************************************************************
                          RTPInterfacePacket.h  -  description
                             -------------------
    begin                : Fri Oct 19 2001
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

/** \file RTPInterfacePacket.h
 * This file declares the class RTPInterfacePacket. This class is derived from
 * cPacket and is used for controlling the rtp layer by the rtp application.
 */


#ifndef __INET_RTPINTERFACEPACKET_H
#define __INET_RTPINTERFACEPACKET_H

#include "INETDefs.h"
#include "IPAddress.h"
#include "RTPSenderControlMessage_m.h"
#include "RTPSenderStatusMessage_m.h"



/**
 * The class RTPInterfacePacket is used for communication between an RTPApplication
 * and an RTPLayer module. Its offers functionality for starting and stopping of an
 * rtp session, transmission of files and feedback about the success of the
 * operations.
 */
class INET_API RTPInterfacePacket : public cPacket
{

    public:

        /**
         * An enumeration to distinguish the different functions of the
         * RTPInterfacePacket.
         */
        enum RTP_IFP_TYPE {
            RTP_IFP_UNDEF,
            RTP_IFP_ENTER_SESSION,
            RTP_IFP_SESSION_ENTERED,
            RTP_IFP_CREATE_SENDER_MODULE,
            RTP_IFP_SENDER_MODULE_CREATED,
            RTP_IFP_DELETE_SENDER_MODULE,
            RTP_IFP_SENDER_MODULE_DELETED,
            RTP_IFP_SENDER_CONTROL,
            RTP_IFP_SENDER_STATUS,
            RTP_IFP_LEAVE_SESSION,
            RTP_IFP_SESSION_LEFT
        };

        /**
         * Default constructor.
         */
        RTPInterfacePacket(const char *name = NULL);

        /**
         * Copy constructor.
         */
        RTPInterfacePacket(const RTPInterfacePacket& rifp);

        /**
         * Destructor.
         */
        virtual ~RTPInterfacePacket();

        /**
         * Assignment operator.
         */
        RTPInterfacePacket& operator=(const RTPInterfacePacket& rifp);

        /**
         * Duplicates the RTPInterfacePacket by calling the copy constructor.
         */
        virtual RTPInterfacePacket *dup() const;

        /**
         * Writes a one line info about this RTPInterfacePacket into the given string.
         */
        virtual std::string info();

        /**
         * Writes a longer info about this RTPInterfacePacket into the given stream.
         */
        virtual void dump(std::ostream& os);

        /**
         * Called by the rtp application to make the rtp layer enter an
         * rtp session with the given parameters.
         */
        virtual void enterSession(const char *commonName, const char *profileName, int bandwidth, IPAddress destinationAddress, int port);

        /**
         * Called by the rtp module to inform the application that the rtp session
         * has been entered.
         */
        virtual void sessionEntered(uint32 ssrc);


        virtual void createSenderModule(uint32 ssrc, int payloadType, const char *fileName);
        virtual void senderModuleCreated(uint32 ssrc);
        virtual void deleteSenderModule(uint32 ssrc);
        virtual void senderModuleDeleted(uint32 ssrc);
        virtual void senderModuleControl(uint32 ssrc, RTPSenderControlMessage *msg);
        virtual void senderModuleStatus(uint32 ssrc, RTPSenderStatusMessage *msg);

        /**
         * Called by the application to order the rtp layer to start
         * transmitting a file.
         */
        //virtual void startTransmission(uint32 ssrc, int payloadType, const char *fileName);

        /**
         * Called by the rtp module to inform the application that
         * the transmitting has begun.
         */
        //virtual void transmissionStarted(uint32 ssrc);

        /**
         * Called by the rtp module to inform the application
         * that the transmission has been finished because the
         * end of the file has been reached.
         */
        //virtual void transmissionFinished(uint32 ssrc);

        /**
         * Called by the application to order the rtp layer to
         * stop transmitting.
         */
        //virtual void stopTransmission(uint32 ssrc);

        /**
         * Called by the rtp module to inform the application that
         * the transmission has been stopped as ordered.
         */
        //virtual void transmissionStopped(uint32 ssrc);

        /**
         * Called by the application to order the rtp layer to
         * stop participating in this rtp session.
         */
        virtual void leaveSession();

        /**
         * Called by the rtp module to inform the application
         * that this end system stop participating in this
         * rtp session.
         */
        virtual void sessionLeft();

        /**
         * Returns the type of this RTPInterfacePacket.
         */
        virtual RTP_IFP_TYPE getType();

        /**
         * Returns the CNAME stored in this RTPInterfacePacket.
         */
        virtual const char *getCommonName();

        /**
         * Returns the profile name stored in this RTPInterfacePacket.
         */
        virtual const char *getProfileName();

        /**
         * Returns the bandidth stored in this RTPInterfacePacket.
         */
        virtual int getBandwidth();

        /**
         * Returns the address stored in this RTPInterfacePacket.
         */
        virtual IPAddress getDestinationAddress();

        /**
         * Returns the port stored in this RTPInterfacePacket.
         */
        virtual int getPort();

        /**
         * Returns the ssrc identifier stored in this RTPInterfacePacket.
         */
        virtual uint32 getSSRC();

        /**
         * Returns the payload type stored in this RTPInterfacePacket.
         */
        virtual int getPayloadType();

        /**
         * Returns the file name stored in this RTPInterfacePacket.
         */
        virtual const char *getFileName();

    protected:

        /**
         * The type of the RTPInterfacePacket.
         */
        RTP_IFP_TYPE _type;

        /**
         * The CNAME stored in this RTPInterfacePacket.
         */
        const char *_commonName;

        /**
         * The profile name stored in this RTPInterfacePacket.
         */
        const char *_profileName;

        /**
         * The bandwidth stored in this RTPInterfacePacket.
         */
        int _bandwidth;

        /**
         * The address stored in this RTPInterfacePacket.
         */
        IPAddress _destinationAddress;

        /**
         * The port stored in this RTPInterfacePacket.
         */
        int _port;

        /**
         * The ssrc identifier stored in this RTPInterfacePacket.
         */
        uint32 _ssrc;

        /**
         * The payload type stored in this RTPInterfacePacket.
         */
        int _payloadType;

        /**
         * The file name stored in this RTPInterfacePacket.
         */
        const char *_fileName;
};

#endif

