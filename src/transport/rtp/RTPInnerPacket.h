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


#ifndef __INET_RTPINNERPACKET_H
#define __INET_RTPINNERPACKET_H

#include "RTPInnerPacket_m.h"


/**
 * This class is used for communication between submodules of the RTP layer module.
 */
class RTPInnerPacket : public RTPInnerPacket_Base
{
  public:
    RTPInnerPacket(const char *name=NULL, int kind=0) : RTPInnerPacket_Base(name,kind) {}
    RTPInnerPacket(const RTPInnerPacket& other) : RTPInnerPacket_Base(other.getName()) {operator=(other);}
    RTPInnerPacket& operator=(const RTPInnerPacket& other) {RTPInnerPacket_Base::operator=(other); return *this;}
    virtual RTPInnerPacket *dup() const {return new RTPInnerPacket(*this);}

    // ADD CODE HERE to redefine and implement pure virtual functions from RTPInnerPacket_Base
     /**
     * Writes a short info about this RTPInnerPacket into the given string.
     */
    virtual std::string info() const;

    /**
     * Writes a longer info about this RTPInnerPacket into the given output stream.
     */
    virtual void dump(std::ostream& os) const;

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
    virtual void profileInitialized(int rtcpPercentage, int port);

    /**
     * Called by the rtp module to inform the rtcp module about mandatory
     * information for starting the rtp session.
     */
    virtual void initializeRTCP(const char *commonName, int mtu, int bandwidth, int rtcpPercentage, IPAddress address, int port);

    /**
     * Called by the rtcp module after it has waited for half an rtcp interval
     * for incoming messages from other session participants. It informs the rtp
     * module which later informs the rtp application about the ssrc identifier
     */
    virtual void rtcpInitialized(uint32 ssrc);

    virtual void createSenderModule(uint32 ssrc, int payloadType, const char *fileName);
    virtual void senderModuleCreated(uint32 ssrc);

    virtual void deleteSenderModule(uint32 ssrc);
    virtual void senderModuleDeleted(uint32 ssrc);

    virtual void initializeSenderModule(uint32 ssrc, const char *fileName, int mtu);
    virtual void senderModuleInitialized(uint32 ssrc, int payloadType, int clockRate, int timeStampBase, int sequenceNumberBase);

    virtual void senderModuleControl(uint32 ssrc, RTPSenderControlMessage *msg);
    virtual void senderModuleStatus(uint32 ssrc, RTPSenderStatusMessage *msg);

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
    virtual void dataIn(RTPPacket *packet, IPAddress address, int port);

    /**
     * Returns the maximum transmission unit stored in this RTPInnerPacket.
     */
    virtual int getMTU() const { return getMtu(); }

};

#endif
