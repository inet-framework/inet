/***************************************************************************
                          RTPInterfacePacket.cc  -  description
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

/** \file RTPInterfacePacket.cc
 *
 */

#include "RTPInterfacePacket.h"

Register_Class(RTPInterfacePacket);


std::string RTPInterfacePacket::info() const
{
    std::stringstream out;
    out << "RTPInterfacePacket: type=" << type_var;
    return out.str();
}

void RTPInterfacePacket::dump(std::ostream& os) const
{
    os << "RTPInterfacePacket:" << endl;
    os << "  type = " << type_var << endl;
    os << "  commonName = " << commonName_var << endl;
    os << "  profileName = " << profileName_var << endl;
    os << "  bandwidth = " << bandwidth_var << endl;
    os << "  destinationAddress = " << destinationAddress_var << endl;
    os << "  port = " << port_var << endl;
    os << "  ssrc = " << ssrc_var << endl;
    os << "  payloadType = " << payloadType_var << endl;
    os << "  fileName = " << fileName_var << endl;
}

void RTPInterfacePacket::enterSession(const char *commonName, const char *profileName, int bandwidth, IPAddress destinationAddress, int port)
{
    type_var = RTP_IFP_ENTER_SESSION;
    commonName_var = commonName;
    profileName_var = profileName;
    bandwidth_var = bandwidth;
    destinationAddress_var = destinationAddress;
    port_var = port;
}

void RTPInterfacePacket::sessionEntered(uint32 ssrc)
{
    type_var = RTP_IFP_SESSION_ENTERED;
    ssrc_var = ssrc;
}

void RTPInterfacePacket::createSenderModule(uint32 ssrc, int payloadType, const char *fileName)
{
    type_var = RTP_IFP_CREATE_SENDER_MODULE;
    ssrc_var = ssrc;
    payloadType_var =payloadType;
    fileName_var = fileName;
}

void RTPInterfacePacket::senderModuleCreated(uint32 ssrc)
{
    type_var = RTP_IFP_SENDER_MODULE_CREATED;
    ssrc_var = ssrc;
}

void RTPInterfacePacket::deleteSenderModule(uint32 ssrc)
{
    type_var = RTP_IFP_DELETE_SENDER_MODULE;
    ssrc_var = ssrc;
}

void RTPInterfacePacket::senderModuleDeleted(uint32 ssrc)
{
    type_var = RTP_IFP_SENDER_MODULE_DELETED;
    ssrc_var = ssrc;
}

void RTPInterfacePacket::senderModuleControl(uint32 ssrc, RTPSenderControlMessage *msg)
{
    type_var = RTP_IFP_SENDER_CONTROL;
    ssrc_var = ssrc;
    encapsulate(msg);
}

void RTPInterfacePacket::senderModuleStatus(uint32 ssrc, RTPSenderStatusMessage *msg)
{
    type_var = RTP_IFP_SENDER_STATUS;
    ssrc_var = ssrc;
    encapsulate(msg);
}

/*
void RTPInterfacePacket::startTransmission(uint32 ssrc, int payloadType, const char *fileName)
{
    type_var = RTP_IFP_START_TRANSMISSION;
    ssrc_var = ssrc;
    payloadType_var = payloadType;
    fileName_var = fileName;
}

void RTPInterfacePacket::transmissionStarted(uint32 ssrc)
{
    type_var = RTP_IFP_TRANSMISSION_STARTED;
    ssrc_var = ssrc;
}

void RTPInterfacePacket::transmissionFinished(uint32 ssrc)
{
    type_var = RTP_IFP_TRANSMISSION_FINISHED;
    ssrc_var = ssrc;
}

void RTPInterfacePacket::stopTransmission(uint32 ssrc)
{
    type_var = RTP_IFP_STOP_TRANSMISSION;
    ssrc_var = ssrc;
}

void RTPInterfacePacket::transmissionStopped(uint32 ssrc)
{
    type_var = RTP_IFP_TRANSMISSION_STOPPED;
    ssrc_var = ssrc;
}
*/

void RTPInterfacePacket::leaveSession()
{
    type_var = RTP_IFP_LEAVE_SESSION;
}

void RTPInterfacePacket::sessionLeft()
{
    type_var = RTP_IFP_SESSION_LEFT;
}

