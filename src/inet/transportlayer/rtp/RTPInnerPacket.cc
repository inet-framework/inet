/***************************************************************************
                          RTPInnerPacket.cc  -  description
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

#include "inet/transportlayer/rtp/RTPInnerPacket.h"

#include "inet/transportlayer/rtp/RTPPacket.h"

namespace inet {

namespace rtp {

Register_Class(RTPInnerPacket);

std::string RTPInnerPacket::str() const
{
    std::stringstream out;
    out << "RTPInnerPacket: type=" << type;
    return out.str();
}

void RTPInnerPacket::dump(std::ostream& os) const
{
    os << "RTPInnerPacket:" << endl;
    os << "  type = " << type << endl;
    os << "  commonName = " << commonName << endl;
    os << "  mtu = " << mtu << endl;
    os << "  bandwidth = " << bandwidth << endl;
    os << "  rtcpPercentage = " << rtcpPercentage << endl;
    os << "  address = " << address << endl;
    os << "  port = " << port << endl;
    os << "  ssrc = " << ssrc << endl;
    os << "  payloadType = " << payloadType << endl;
    os << "  fileName = " << fileName << endl;
    os << "  clockRate = " << clockRate << endl;
    os << "  timeStampBase = " << timeStampBase << endl;
    os << "  sequenceNumberBase = " << sequenceNumberBase << endl;
}

void RTPInnerPacket::setInitializeProfilePkt(int mtu_par)
{
    type = RTP_INP_INITIALIZE_PROFILE;
    mtu = mtu_par;
}

void RTPInnerPacket::setProfileInitializedPkt(int rtcpPercentage_par, int port_par)
{
    type = RTP_INP_PROFILE_INITIALIZED;
    rtcpPercentage = rtcpPercentage_par;
    port = port_par;
}

void RTPInnerPacket::setInitializeRTCPPkt(const char *commonName_par, int mtu_par, int bandwidth_par, int rtcpPercentage_par, IPv4Address address_par, int port_par)
{
    type = RTP_INP_INITIALIZE_RTCP;
    commonName = commonName_par;
    mtu = mtu_par;
    bandwidth = bandwidth_par;
    rtcpPercentage = rtcpPercentage_par;
    address = address_par;
    port = port_par;
}

void RTPInnerPacket::setRtcpInitializedPkt(uint32 ssrc_par)
{
    type = RTP_INP_RTCP_INITIALIZED;
    ssrc = ssrc_par;
}

void RTPInnerPacket::setCreateSenderModulePkt(uint32 ssrc_par, int payloadType_par, const char *fileName_par)
{
    type = RTP_INP_CREATE_SENDER_MODULE;
    ssrc = ssrc_par;
    payloadType = payloadType_par;
    fileName = fileName_par;
}

void RTPInnerPacket::setSenderModuleCreatedPkt(uint32 ssrc_par)
{
    type = RTP_INP_SENDER_MODULE_CREATED;
    ssrc = ssrc_par;
}

void RTPInnerPacket::setDeleteSenderModulePkt(uint32 ssrc_par)
{
    type = RTP_INP_DELETE_SENDER_MODULE;
    ssrc = ssrc_par;
}

void RTPInnerPacket::setSenderModuleDeletedPkt(uint32 ssrc_par)
{
    type = RTP_INP_SENDER_MODULE_DELETED;
    ssrc = ssrc_par;
}

void RTPInnerPacket::setInitializeSenderModulePkt(uint32 ssrc_par, const char *fileName_par, int mtu_par)
{
    type = RTP_INP_INITIALIZE_SENDER_MODULE;
    ssrc = ssrc_par;
    fileName = fileName_par;
    mtu = mtu_par;
}

void RTPInnerPacket::setSenderModuleInitializedPkt(uint32 ssrc_par, int payloadType_par, int clockRate_par, int timeStampBase_par, int sequenceNumberBase_par)
{
    type = RTP_INP_SENDER_MODULE_INITIALIZED;
    ssrc = ssrc_par;
    payloadType = payloadType_par;
    clockRate = clockRate_par;
    timeStampBase = timeStampBase_par;
    sequenceNumberBase = sequenceNumberBase_par;
}

void RTPInnerPacket::setSenderModuleControlPkt(uint32 ssrc_par, RTPSenderControlMessage *msg)
{
    type = RTP_INP_SENDER_MODULE_CONTROL;
    ssrc = ssrc_par;
    encapsulate(msg);
}

void RTPInnerPacket::setSenderModuleStatusPkt(uint32 ssrc_par, RTPSenderStatusMessage *msg)
{
    type = RTP_INP_SENDER_MODULE_STATUS;
    ssrc = ssrc_par;
    encapsulate(msg);
}

void RTPInnerPacket::setLeaveSessionPkt()
{
    type = RTP_INP_LEAVE_SESSION;
}

void RTPInnerPacket::setSessionLeftPkt()
{
    type = RTP_INP_SESSION_LEFT;
}

void RTPInnerPacket::setDataOutPkt(RTPPacket *packet)
{
    type = RTP_INP_DATA_OUT;
    encapsulate(packet);
}

void RTPInnerPacket::setDataInPkt(RTPPacket *packet, IPv4Address address_par, int port_par)
{
    type = RTP_INP_DATA_IN;
    address = address_par;
    port = port_par;
    encapsulate(packet);
}

} // namespace rtp

} // namespace inet

