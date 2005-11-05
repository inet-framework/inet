/***************************************************************************
                          RTPPayloadReceiver.h  -  description
                             -------------------
    begin                : Fri Nov 2 2001
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


/** \file RTPPayloadReceiver.h
 * This file declares the class RTPPayloadReceiver.
 */

#ifndef __RTPPAYLOADRECEIVER_H__
#define __RTPPAYLOADRECEIVER_H__


#include <fstream>
#include <omnetpp.h>
#include "INETDefs.h"
#include "RTPPacket.h"


/**
 * The class RTPPayloadReceiver acts as a base class for modules
 * processing incoming rtp data packets.
 */
class INET_API RTPPayloadReceiver : public cSimpleModule
{

  protected:

    /**
     * Destructor. Disposes the queue object and closes the output file.
     */
    virtual ~RTPPayloadReceiver();

    /**
     * Initializes the receiver module, opens the output file and creates
     * a queue for incoming packets.
     * Subclasses must overwrite it (but should call this method too)
     */
    virtual void initialize();

    /**
     * Method for handling incoming packets. At the moment only RTPInnerPackets
     * containing an encapsulated RTPPacket are handled.
     */
    virtual void handleMessage(cMessage *msg);

    protected:

        /**
         * The output file stream.
         */
        std::ofstream _outputFileStream;

        /**
         * The payload type this RTPPayloadReceiver module processes.
         */
        int _payloadType;

        /**
         * An output vector used to store arrival of rtp data packets.
         */
        cOutVector *_packetArrival;

        /**
         * Writes contents of this RTPPacket into the output file. Must be overwritten
         * by subclasses.
         */
        virtual void processPacket(RTPPacket *packet);

        /**
         * This method is called by initialize and opens the output file stream.
         * For most payload receivers this method works well, only when using
         * a library for a payload type which provides an own open method it must
         */
        virtual void openOutputFile(const char *fileName);

        /**
         * Closes the output file stream.
         */
        virtual void closeOutputFile();
};

#endif

