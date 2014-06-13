/***************************************************************************
                       RTPPayloadReceiver.h  -  description
                             -------------------
    begin            : Fri Aug 2 2007
    copyright        : (C) 2007 by Matthias Oppitz, Ahmed Ayadi
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


#ifndef __INET_RTPPAYLOADRECEIVER_H
#define __INET_RTPPAYLOADRECEIVER_H


#include <fstream>

#include "INETDefs.h"


//Forward declarations
class RTPPacket;


/**
 * The class RTPPayloadReceiver acts as a base class for modules
 * processing incoming RTP data packets.
 */
class INET_API RTPPayloadReceiver : public cSimpleModule
{
  public:
    /**
     * Destructor. Disposes the queue object and closes the output file.
     */
    virtual ~RTPPayloadReceiver();

  protected:
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

  protected:
   /**
     * The output file stream.
     */
    std::ofstream _outputFileStream;

    /**
     * The output file stream.
     */
    std::ofstream _outputLogLoss;

    /**
     * The payload type this RTPPayloadReceiver module processes.
     */
    int _payloadType;

    /**
     * An output signal used to store arrival of rtp data packets.
     */
    static simsignal_t _rcvdPkRtpTimestampSignal;
};

#endif
