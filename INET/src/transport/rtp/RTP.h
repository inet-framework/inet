/***************************************************************************
                       RTP.h  -  description
                             -------------------
    begin            : Fri Aug 2 2007
    copyright        : (C) 2007 by Matthias Oppitz,  Ahmed Ayadi
    email            : <Matthias.Oppitz@gmx.de> <ahmed.ayadi@sophia.inria.fr>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/** \file RTP.h
 * This header file declares the class RTP.
 */

#ifndef __INET_RTPENDSYSTEMMODULE_H
#define __INET_RTPENDSYSTEMMODULE_H

#include "INETDefs.h"
#include "IPvXAddress.h"
#include "RTPInterfacePacket.h"
#include "RTPInnerPacket.h"

/**
 * An RTP is the center of the rtp layer of an endsystem.
 * It creates the profile module, sends and receives rtp data packets
 * and forwards messages.
 * It also communicates with the application.
 */
class INET_API RTP : public cSimpleModule
{
    protected:

        /**
         * Initializes variables.
         */
        virtual void initialize();

        /**
         * Handles incoming messages.
         */
        virtual void handleMessage(cMessage *msg);

    protected:

        /**
         * Handles messages received from the applicaiton.
         */
        virtual void handleMessageFromApp(cMessage *msg);

        /**
         * Handles messages received from the profile module.
         */
        virtual void handleMessageFromProfile(cMessage *msg);

        /**
         * Handles messages received from the rtcp module.
         */
        virtual void handleMessageFromRTCP(cMessage *msg);

        /**
         * Handles messages received from the UDP layer.
         */
        virtual void handleMessagefromUDP(cMessage *msg);

        /**
         * Creates the profile module and initializes it.
         */
        virtual void enterSession(RTPInterfacePacket *rifp);

        /**
         * Destroys the profile module and orders the rtcp module
         * to send an rtcp bye packet.
         */
        virtual void leaveSession(RTPInterfacePacket *rifp);

        virtual void createSenderModule(RTPInterfacePacket *rifp);
        virtual void deleteSenderModule(RTPInterfacePacket *rifp);
        virtual void senderModuleControl(RTPInterfacePacket *rifp);


        /**
         * Called when the profile module is initialized.
         */
        virtual void profileInitialized(RTPInnerPacket *rinp);

        virtual void senderModuleCreated(RTPInnerPacket *rinp);
        virtual void senderModuleDeleted(RTPInnerPacket *rinp);
        virtual void senderModuleInitialized(RTPInnerPacket *rinp);
        virtual void senderModuleStatus(RTPInnerPacket *rinp);

        /**
         * Sends a rtp data packet to the UDP layer and a copy
         * of it to the rtcp module.
         */
        virtual void dataOut(RTPInnerPacket *rinp);

        /**
         * Informs the application that the session is entered.
         */
        virtual void rtcpInitialized(RTPInnerPacket *rinp);

        /**
         * Informs the application that this end system
         * has left the rtp session.
         */
        virtual void sessionLeft(RTPInnerPacket *rinp);


    protected:
        /**
         * The CNAME of this end system.
         */
        const char *_commonName;

        /**
         * The name of the profile used in this session.
         */
        const char *_profileName;

        /**
         * The available bandwidth for this session.
         */
        int _bandwidth;

        /**
         * The destination address.
         */
        IPAddress _destinationAddress;

        /**
         * The rtp port.
         */
        int _port;

        /**
         * The maximum size of a packet.
         */
        int _mtu;

        /**
         * The percentage of the bandwidth used for rtcp.
         */
        int _rtcpPercentage;

        /**
         * The rtp server socket file descriptor.
         */
        int _socketFdIn;

        /**
         * The rtp client socket file descriptor.
         */
        int _socketFdOut;

        /**
         * True when this end system is about to leave the session.
         */
        bool _leaveSession;

        /**
         * Creates the profile module.
         */
        virtual void createProfile();

        /**
         * Requests a server socket from the UDP layer.
         */
        virtual void createSocket();

        /**
         * Called when the socket layer returns a socket.
         */
        virtual void socketRet();

        /**
         * Called when the socket layer has connected a socket.
         */
        virtual void connectRet();

        /**
         * Called when data from the socket layer has been received.
         */
        virtual void readRet(cMessage *sifp);

        /**
         * Initializes the profile module.
         */
        virtual void initializeProfile();

        /**
         * Initializes the rtcp module-.
         */
        virtual void initializeRTCP();

        /**
        Determines the maximum transmission unit that can be uses for
        rtp. This implementation assumes that we use an ethernet with
        1500 bytes mtu. The returned value is 1500 bytes minus header
        sizes for ip and udp.
        */
        virtual int resolveMTU();
};

#endif

