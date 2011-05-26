/***************************************************************************
                       RTPApplication.h  -  description
                             -------------------
    (C) 2007 Ahmed Ayadi  <ahmed.ayadi@sophia.inria.fr>
    (C) 2001 Matthias Oppitz <Matthias.Oppitz@gmx.de>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __INET_RTPAPPLICATION_H
#define __INET_RTPAPPLICATION_H

#include "INETDefs.h"

#include "IPv4Address.h"

/**
 * The class RTPApplication is just a very simple sample for an application
 * which uses RTP. It acts as a sender if the omnet parameter fileName is
 * set, and as a receiver if the parameter is empty.
 */
class INET_API RTPApplication : public cSimpleModule
{
    public:
        /**
         * Constructor, with activity() stack size.
         */
        RTPApplication() : cSimpleModule() {}

    protected:
        /**
         * Reads the OMNeT++ parameters.
         */
        virtual void initialize(int stage);
        virtual int numInitStages() const {return 4;}

        /**
         * RTPApplication uses activity for message handling.
         * The behaviour is controlled by omnet parameters.
         */
        virtual void handleMessage(cMessage* msg);

    protected:
        enum SelfMsgKind
        {
            ENTER_SESSION,
            START_TRANSMISSION,
            STOP_TRANSMISSION,
            LEAVE_SESSION
        };
        /**
         * The CNAME of this participant.
         */
        const char *_commonName;

        /**
         * The name of the used profile.
         */
        const char *_profileName;

        /**
         * The reserved bandwidth for rtp/rtcp in bytes/second.
         */
        int _bandwidth;

        /**
         * The address of the unicast peer or of the multicast group.
         */
        IPv4Address _destinationAddress;

        /**
         * One of the udp port used.
         */
        int _port;

        /**
         * The name of the file to be transmitted.
         */
        const char *_fileName;

        /**
         * The payload type of the data in the file.
         */
        int _payloadType;

        /**
         * The delay after the application enters the session,
         */
        simtime_t _sessionEnterDelay;

        /**
         * The delay after the application starts the transmission.
         */
        simtime_t _transmissionStartDelay;

        /**
         * The delay after the application stops the transmission.
         */
        simtime_t _transmissionStopDelay;

        /**
         * The delay after the application leaves the session.
         */
        simtime_t _sessionLeaveDelay;

        uint32 ssrc;

        bool isActiveSession;
};

#endif
