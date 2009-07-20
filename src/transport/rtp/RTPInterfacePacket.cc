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


RTPInterfacePacket::RTPInterfacePacket(const char *name) : cPacket(name)
{
    _type = RTP_IFP_UNDEF;
    _commonName = NULL;
    _profileName = NULL;
    _bandwidth = 0;
    _destinationAddress = IPAddress::UNSPECIFIED_ADDRESS;
    _port = PORT_UNDEF;
    _ssrc = 0;
    _payloadType = 0;
    _fileName = NULL;
}


RTPInterfacePacket::RTPInterfacePacket(const RTPInterfacePacket& rifp) : cPacket()
{
    setName(rifp.getName());
    operator=(rifp);
}


RTPInterfacePacket::~RTPInterfacePacket()
{
    if (opp_strcmp(_commonName, ""))
        delete _commonName;
    if (opp_strcmp(_profileName, ""))
        delete _profileName;
    if (opp_strcmp(_fileName, ""))
        delete _fileName;
}


RTPInterfacePacket& RTPInterfacePacket::operator=(const RTPInterfacePacket& rifp)
{
    cPacket::operator=(rifp);
    _type = rifp._type;
    _commonName = opp_strdup(rifp._commonName);
    _profileName = opp_strdup(rifp._profileName);
    _bandwidth = rifp._bandwidth;
    _destinationAddress = rifp._destinationAddress;
    _port = rifp._port;
    _ssrc = rifp._ssrc;
    _payloadType = rifp._payloadType;
    _fileName = opp_strdup(rifp._fileName);
    return *this;
}


RTPInterfacePacket *RTPInterfacePacket::dup() const
{
    return new RTPInterfacePacket(*this);
}


std::string RTPInterfacePacket::info()
{
    std::stringstream out;
    out << "RTPInterfacePacket: type=" << _type;
    return out.str();
}


void RTPInterfacePacket::dump(std::ostream& os)
{
    os << "RTPInterfacePacket:" << endl;
    os << "  type = " << _type << endl;
    os << "  commonName = " << _commonName << endl;
    os << "  profileName = " << _profileName << endl;
    os << "  bandwidth = " << _bandwidth << endl;
    os << "  destinationAddress = " << _destinationAddress << endl;
    os << "  port = " << _port << endl;
    os << "  ssrc = " << _ssrc << endl;
    os << "  payloadType = " << _payloadType << endl;
    os << "  fileName = " << _fileName << endl;
}


void RTPInterfacePacket::enterSession(const char *commonName, const char *profileName, int bandwidth, IPAddress destinationAddress, int port)
{
    _type = RTP_IFP_ENTER_SESSION;
    _commonName = commonName;
    _profileName = profileName;
    _bandwidth = bandwidth;
    _destinationAddress = destinationAddress;
    _port = port;
}


void RTPInterfacePacket::sessionEntered(uint32 ssrc)
{
    _type = RTP_IFP_SESSION_ENTERED;
    _ssrc = ssrc;
}


void RTPInterfacePacket::createSenderModule(uint32 ssrc, int payloadType, const char *fileName)
{
    _type = RTP_IFP_CREATE_SENDER_MODULE;
    _ssrc = ssrc;
    _payloadType =payloadType;
    _fileName = fileName;
}


void RTPInterfacePacket::senderModuleCreated(uint32 ssrc)
{
    _type = RTP_IFP_SENDER_MODULE_CREATED;
    _ssrc = ssrc;
}


void RTPInterfacePacket::deleteSenderModule(uint32 ssrc)
{
    _type = RTP_IFP_DELETE_SENDER_MODULE;
    _ssrc = ssrc;
}


void RTPInterfacePacket::senderModuleDeleted(uint32 ssrc)
{
    _type = RTP_IFP_SENDER_MODULE_DELETED;
    _ssrc = ssrc;
}


void RTPInterfacePacket::senderModuleControl(uint32 ssrc, RTPSenderControlMessage *msg)
{
    _type = RTP_IFP_SENDER_CONTROL;
    _ssrc = ssrc;
    encapsulate(msg);
}


void RTPInterfacePacket::senderModuleStatus(uint32 ssrc, RTPSenderStatusMessage *msg)
{
    _type = RTP_IFP_SENDER_STATUS;
    _ssrc = ssrc;
    encapsulate(msg);
}

/*
void RTPInterfacePacket::startTransmission(uint32 ssrc, int payloadType, const char *fileName)
{
    _type = RTP_IFP_START_TRANSMISSION;
    _ssrc = ssrc;
    _payloadType = payloadType;
    _fileName = fileName;
}


void RTPInterfacePacket::transmissionStarted(uint32 ssrc)
{
    _type = RTP_IFP_TRANSMISSION_STARTED;
    _ssrc = ssrc;
}


void RTPInterfacePacket::transmissionFinished(uint32 ssrc)
{
    _type = RTP_IFP_TRANSMISSION_FINISHED;
    _ssrc = ssrc;
}


void RTPInterfacePacket::stopTransmission(uint32 ssrc)
{
    _type = RTP_IFP_STOP_TRANSMISSION;
    _ssrc = ssrc;
}


void RTPInterfacePacket::transmissionStopped(uint32 ssrc)
{
    _type = RTP_IFP_TRANSMISSION_STOPPED;
    _ssrc = ssrc;
}
*/


void RTPInterfacePacket::leaveSession()
{
    _type = RTP_IFP_LEAVE_SESSION;
}


void RTPInterfacePacket::sessionLeft()
{
    _type = RTP_IFP_SESSION_LEFT;
}


RTPInterfacePacket::RTP_IFP_TYPE RTPInterfacePacket::getType()
{
    return _type;
}


const char *RTPInterfacePacket::getCommonName()
{
    return opp_strdup(_commonName);
}


const char *RTPInterfacePacket::getProfileName()
{
    return opp_strdup(_profileName);
}


uint32 RTPInterfacePacket::getSSRC()
{
    return _ssrc;
}


int RTPInterfacePacket::getPayloadType()
{
    return _payloadType;
}


const char *RTPInterfacePacket::getFileName()
{
    return opp_strdup(_fileName);
}


int RTPInterfacePacket::getBandwidth()
{
    return _bandwidth;
}


IPAddress RTPInterfacePacket::getDestinationAddress()
{
    return _destinationAddress;
}


int RTPInterfacePacket::getPort()
{
    return _port;
}
