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

#include "RTPInterfacePacket_m.h"

/**
 * The class RTPInterfacePacket is used for communication between an RTPApplication
 * and an RTPLayer module. Its offers functionality for starting and stopping of an
 * rtp session, transmission of files and feedback about the success of the
 * operations.
 */
class RTPInterfacePacket : public RTPInterfacePacket_Base
{
  public:
    RTPInterfacePacket(const char *name=NULL, int kind=0) : RTPInterfacePacket_Base(name,kind) {}
    RTPInterfacePacket(const RTPInterfacePacket& other) : RTPInterfacePacket_Base(other.getName()) {operator=(other);}
    RTPInterfacePacket& operator=(const RTPInterfacePacket& other) {RTPInterfacePacket_Base::operator=(other); return *this;}
    virtual RTPInterfacePacket *dup() const {return new RTPInterfacePacket(*this);}
    // ADD CODE HERE to redefine and implement pure virtual functions from RTPInterfacePacket_Base

  public:
    /**
     * Writes a one line info about this RTPInterfacePacket into the given string.
     */
    virtual std::string info() const;

    /**
     * Writes a longer info about this RTPInterfacePacket into the given stream.
     */
    virtual void dump(std::ostream& os) const;

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
};

#endif
