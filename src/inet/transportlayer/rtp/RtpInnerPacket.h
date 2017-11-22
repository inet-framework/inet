/***************************************************************************
                          RtpInnerPacket.h  -  description
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

#include "inet/common/packet/Packet.h"
#include "inet/transportlayer/rtp/RtpInnerPacket_m.h"

namespace inet {

namespace rtp {

/**
 * This class is used for communication between submodules of the RTP layer module.
 */
class INET_API RtpInnerPacket : public RtpInnerPacket_Base
{
  public:
    RtpInnerPacket(const char *name = nullptr, int kind = 0) : RtpInnerPacket_Base(name, kind) {}
    RtpInnerPacket(const RtpInnerPacket& other) : RtpInnerPacket_Base(other) {}
    RtpInnerPacket& operator=(const RtpInnerPacket& other) { RtpInnerPacket_Base::operator=(other); return *this; }
    virtual RtpInnerPacket *dup() const override { return new RtpInnerPacket(*this); }

    // ADD CODE HERE to redefine and implement pure virtual functions from RtpInnerPacket_Base
    /**
     * Writes a short info about this RtpInnerPacket into the given string.
     */
    virtual std::string info() const override;

    /**
     * Writes a longer info about this RtpInnerPacket into the given output stream.
     */
    virtual void dump(std::ostream& os) const;

    /**
     * Called by the rtp module after creating the profile module. It
     * informes the profile about the maximum size an rtp packet can have.
     */
    virtual void setInitializeProfilePkt(int mtu);

    /**
     * Called by the profile module after it has received the initializeProfile()
     * message. It informs the rtp module about the percentage of the available
     * bandwidth to be used by rtcp and the preferred port for this profile.
     */
    virtual void setProfileInitializedPkt(int rtcpPercentage, int port);

    /**
     * Called by the rtp module to inform the rtcp module about mandatory
     * information for starting the rtp session.
     */
    virtual void setInitializeRTCPPkt(const char *commonName, int mtu, int bandwidth,
            int rtcpPercentage, Ipv4Address address, int port);

    /**
     * Called by the rtcp module after it has waited for half an rtcp interval
     * for incoming messages from other session participants. It informs the rtp
     * module which later informs the rtp application about the ssrc identifier
     */
    virtual void setRtcpInitializedPkt(uint32 ssrc);

    virtual void setCreateSenderModulePkt(uint32 ssrc, int payloadType, const char *fileName);
    virtual void setSenderModuleCreatedPkt(uint32 ssrc);

    virtual void setDeleteSenderModulePkt(uint32 ssrc);
    virtual void setSenderModuleDeletedPkt(uint32 ssrc);

    virtual void setInitializeSenderModulePkt(uint32 ssrc, const char *fileName, int mtu);
    virtual void setSenderModuleInitializedPkt(uint32 ssrc, int payloadType, int clockRate,
            int timeStampBase, int sequenceNumberBase);

    virtual void setSenderModuleControlPkt(uint32 ssrc, RtpSenderControlMessage *msg);
    virtual void setSenderModuleStatusPkt(uint32 ssrc, RtpSenderStatusMessage *msg);

    /**
     * Called by the rtp module to inform the rtcp module that the session
     * should be left.
     */
    virtual void setLeaveSessionPkt();

    /**
     * Called by the rtcp module when the rtcp bye packet has been sent
     * to the network.
     */
    virtual void setSessionLeftPkt();

    /**
     * Capsulates the outgoing RtpPacket into this RtpInnerPacket to transport
     * it within the rtp layer.
     */
    virtual void setDataOutPkt(Packet *packet);

    /**
     * Capsulates the incoming RtpPacket into this RtpInnerPacket to transport
     * it within the rtp layer.
     */
    virtual void setDataInPkt(Packet *packet, Ipv4Address address, int port);

    /**
     * Returns the maximum transmission unit stored in this RtpInnerPacket.
     */
    virtual int getMTU() const { return getMtu(); }
};

} // namespace rtp

} // namespace inet

#endif // ifndef __INET_RTPINNERPACKET_H

