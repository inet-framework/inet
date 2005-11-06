/***************************************************************************
                          RTPApplication.h  -  description
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

/** \file RTPApplication.h
 * The header file RTPApplication.h declares the generic class RTPApplication
 * for a very simple application which uses the realDelay transport protocol.
 */

#ifndef __RTPAPPLICATION_H__
#define __RTPAPPLICATION_H__

#include <omnetpp.h>
#include "INETDefs.h"

/**
 * The class RTPApplication is just a very simple sample for an application which
 * uses RTP. It acts as a sender if the omnet parameter fileName is set, and as
 * a receiver if the parameter is empty.
 */
class INET_API RTPApplication : public cSimpleModule
{
    protected:
        /**
         * Constructor, with activity() stack size.
         */
        RTPApplication() : cSimpleModule(32768) {}

        /**
         * Reads the OMNeT++ parameters.
         */
        virtual void initialize();

        /**
         * RTPApplication uses activity for message handling.
         * The behaviour is controlled by omnet parameters.
         */
        virtual void activity();

    private:

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
        IN_Addr _destinationAddress;

        /**
         * One of the udp port used.
         */
        IN_Port _port;

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

};

#endif


