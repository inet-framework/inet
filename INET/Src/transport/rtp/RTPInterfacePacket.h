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


#ifndef __RTPINTERFACEPACKET_H__
#define __RTPINTERFACEPACKET_H__

#include <omnetpp.h>
#include "INETDefs.h"

//XXX #include "in_addr.h"
//XXX #include "in_port.h"
#include "tmp/defs.h"

#include "types.h"

#include "RTPSenderControlMessage.h"
#include "RTPSenderStatusMessage.h"



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
        virtual cObject *dup() const;

        /**
         * Returns the class name "RTPInterfacePacket".
         */
        virtual const char *className() const;

        /**
         * Writes a one line info about this RTPInterfacePacket into the given string.
         */
        virtual std::string info();

        /**
         * Writes a longer info about this RTPInterfacePacket into the given stream.
         */
        virtual void writeContents(std::ostream& os);

        /**
         * Called by the rtp application to make the rtp layer enter an
         * rtp session with the given parameters.
         */
        virtual void enterSession(const char *commonName, const char *profileName, int bandwidth, IN_Addr destinationAddress, IN_Port port);

        /**
         * Called by the rtp module to inform the application that the rtp session
         * has been entered.
         */
        virtual void sessionEntered(u_int32 ssrc);


        virtual void createSenderModule(u_int32 ssrc, int payloadType, const char *fileName);
        virtual void senderModuleCreated(u_int32 ssrc);
        virtual void deleteSenderModule(u_int32 ssrc);
        virtual void senderModuleDeleted(u_int32 ssrc);
        virtual void senderModuleControl(u_int32 ssrc, RTPSenderControlMessage *msg);
        virtual void senderModuleStatus(u_int32 ssrc, RTPSenderStatusMessage *msg);

        /**
         * Called by the application to order the rtp layer to start
         * transmitting a file.
         */
        //virtual void startTransmission(u_int32 ssrc, int payloadType, const char *fileName);

        /**
         * Called by the rtp module to inform the application that
         * the transmitting has begun.
         */
        //virtual void transmissionStarted(u_int32 ssrc);

        /**
         * Called by the rtp module to inform the application
         * that the transmission has been finished because the
         * end of the file has been reached.
         */
        //virtual void transmissionFinished(u_int32 ssrc);

        /**
         * Called by the application to order the rtp layer to
         * stop transmitting.
         */
        //virtual void stopTransmission(u_int32 ssrc);

        /**
         * Called by the rtp module to inform the application that
         * the transmission has been stopped as ordered.
         */
        //virtual void transmissionStopped(u_int32 ssrc);

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
        virtual RTP_IFP_TYPE type();

        /**
         * Returns the CNAME stored in this RTPInterfacePacket.
         */
        virtual const char *commonName();

        /**
         * Returns the profile name stored in this RTPInterfacePacket.
         */
        virtual const char *profileName();

        /**
         * Returns the bandidth stored in this RTPInterfacePacket.
         */
        virtual int bandwidth();

        /**
         * Returns the address stored in this RTPInterfacePacket.
         */
        virtual IN_Addr destinationAddress();

        /**
         * Returns the port stored in this RTPInterfacePacket.
         */
        virtual IN_Port port();

        /**
         * Returns the ssrc identifier stored in this RTPInterfacePacket.
         */
        virtual u_int32 ssrc();

        /**
         * Returns the payload type stored in this RTPInterfacePacket.
         */
        virtual int payloadType();

        /**
         * Returns the file name stored in this RTPInterfacePacket.
         */
        virtual const char *fileName();

    private:

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
        IN_Addr _destinationAddress;

        /**
         * The port stored in this RTPInterfacePacket.
         */
        IN_Port _port;

        /**
         * The ssrc identifier stored in this RTPInterfacePacket.
         */
        u_int32 _ssrc;

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

