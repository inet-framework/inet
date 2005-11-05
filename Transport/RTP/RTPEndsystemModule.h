/***************************************************************************
                          RTPEndsystemModule.h  -  description
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

/** \file RTPEndsystemModule.h
 * This header file declares the class RTPEndsystemModule.
 */

#ifndef __RTPENDSYSTEMMODULE_H__
#define __RTPENDSYSTEMMODULE_H__

#include <omnetpp.h>
#include "INETDefs.h"

//XXX #include "sockets.h"
//XXX #include "in_addr.h"
//XXX #include "in_port.h"
#include "tmp/defs.h"

#include "RTPInterfacePacket.h"
#include "RTPInnerPacket.h"

/**
 * An RTPEndsystemModule is the center of the rtp layer of an endsystem.
 * It creates the profile module, sends and receives rtp data packets
 * and forwards messages.
 * It also communicates with the application.
 */
class INET_API RTPEndsystemModule : public cSimpleModule
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
         * Handles messages received from the socket layer.
         */
        virtual void handleMessageFromSocketLayer(cMessage *msg);

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
         * Sends a rtp data packet to the socket layer and a copy
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


    private:
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
        IN_Addr _destinationAddress;

        /**
         * The rtp port.
         */
        IN_Port _port;

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
        Socket::Filedesc _socketFdIn;

        /**
         * The rtp client socket file descriptor.
         */
        Socket::Filedesc _socketFdOut;

        /**
         * Creates the profile module.
         */
        virtual void createProfile();

        /**
         * Requests a server socket from the socket layer.
         */
        virtual void createServerSocket();

        /**
         * Requests a client socket from the socket layer.
         */
        virtual void createClientSocket();

        /**
         * Called when the socket layer returns a socket.
         */
        virtual void socketRet(SocketInterfacePacket *sifp);

        /**
         * Called when the socket layer has connected a socket.
         */
        virtual void connectRet(SocketInterfacePacket *sifp);

        /**
         * Called when data from the socket layer has been received.
         */
        virtual void readRet(SocketInterfacePacket *sifp);

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

