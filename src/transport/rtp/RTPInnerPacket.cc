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


#include "RTPInnerPacket.h"

#include "RTPPacket.h"


Register_Class(RTPInnerPacket);

std::string RTPInnerPacket::info() const
{
    std::stringstream out;
    out << "RTPInnerPacket: type=" << type_var;
    return out.str();
}

void RTPInnerPacket::dump(std::ostream& os) const
{
    os << "RTPInnerPacket:" << endl;
    os << "  type = " << type_var << endl;
    os << "  commonName = " << commonName_var << endl;
    os << "  mtu = " << mtu_var << endl;
    os << "  bandwidth = " << bandwidth_var << endl;
    os << "  rtcpPercentage = " << rtcpPercentage_var << endl;
    os << "  address = " << address_var << endl;
    os << "  port = " << port_var << endl;
    os << "  ssrc = " << ssrc_var << endl;
    os << "  payloadType = " << payloadType_var << endl;
    os << "  fileName = " << fileName_var << endl;
    os << "  clockRate = " << clockRate_var << endl;
    os << "  timeStampBase = " << timeStampBase_var << endl;
    os << "  sequenceNumberBase = " << sequenceNumberBase_var << endl;
}

void RTPInnerPacket::setInitializeProfilePkt(int mtu)
{
    type_var = RTP_INP_INITIALIZE_PROFILE;
    mtu_var = mtu;
}

void RTPInnerPacket::setProfileInitializedPkt(int rtcpPercentage, int port)
{
    type_var = RTP_INP_PROFILE_INITIALIZED;
    rtcpPercentage_var = rtcpPercentage;
    port_var = port;
}

void RTPInnerPacket::setInitializeRTCPPkt(const char *commonName, int mtu, int bandwidth, int rtcpPercentage, IPv4Address address, int port)
{
    type_var = RTP_INP_INITIALIZE_RTCP;
    commonName_var = commonName;
    mtu_var = mtu;
    bandwidth_var = bandwidth;
    rtcpPercentage_var = rtcpPercentage;
    address_var = address;
    port_var = port;
}

void RTPInnerPacket::setRtcpInitializedPkt(uint32 ssrc)
{
    type_var = RTP_INP_RTCP_INITIALIZED;
    ssrc_var = ssrc;
}

void RTPInnerPacket::setCreateSenderModulePkt(uint32 ssrc, int payloadType, const char *fileName)
{
    type_var = RTP_INP_CREATE_SENDER_MODULE;
    ssrc_var = ssrc;
    payloadType_var = payloadType;
    fileName_var = fileName;
}

void RTPInnerPacket::setSenderModuleCreatedPkt(uint32 ssrc)
{
    type_var = RTP_INP_SENDER_MODULE_CREATED;
    ssrc_var = ssrc;
}

void RTPInnerPacket::setDeleteSenderModulePkt(uint32 ssrc)
{
    type_var = RTP_INP_DELETE_SENDER_MODULE;
    ssrc_var = ssrc;
}

void RTPInnerPacket::setSenderModuleDeletedPkt(uint32 ssrc)
{
    type_var = RTP_INP_SENDER_MODULE_DELETED;
    ssrc_var = ssrc;
}

void RTPInnerPacket::setInitializeSenderModulePkt(uint32 ssrc, const char *fileName, int mtu)
{
    type_var = RTP_INP_INITIALIZE_SENDER_MODULE;
    ssrc_var = ssrc;
    fileName_var = fileName;
    mtu_var = mtu;
}

void RTPInnerPacket::setSenderModuleInitializedPkt(uint32 ssrc, int payloadType, int clockRate, int timeStampBase, int sequenceNumberBase)
{
    type_var = RTP_INP_SENDER_MODULE_INITIALIZED;
    ssrc_var = ssrc;
    payloadType_var = payloadType;
    clockRate_var = clockRate;
    timeStampBase_var = timeStampBase;
    sequenceNumberBase_var = sequenceNumberBase;
}

void RTPInnerPacket::setSenderModuleControlPkt(uint32 ssrc, RTPSenderControlMessage *msg)
{
    type_var = RTP_INP_SENDER_MODULE_CONTROL;
    ssrc_var = ssrc;
    encapsulate(msg);
}

void RTPInnerPacket::setSenderModuleStatusPkt(uint32 ssrc, RTPSenderStatusMessage *msg)
{
    type_var = RTP_INP_SENDER_MODULE_STATUS;
    ssrc_var = ssrc;
    encapsulate(msg);
}

void RTPInnerPacket::setLeaveSessionPkt()
{
    type_var = RTP_INP_LEAVE_SESSION;
}


void RTPInnerPacket::setSessionLeftPkt()
{
    type_var = RTP_INP_SESSION_LEFT;
}

void RTPInnerPacket::setDataOutPkt(RTPPacket *packet)
{
    type_var = RTP_INP_DATA_OUT;
    encapsulate(packet);
}

void RTPInnerPacket::setDataInPkt(RTPPacket *packet, IPv4Address address, int port)
{
    type_var = RTP_INP_DATA_IN;
    address_var = address;
    port_var = port;
    encapsulate(packet);
}
