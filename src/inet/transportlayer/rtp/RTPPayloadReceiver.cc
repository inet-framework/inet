/***************************************************************************
                          RTPPayloadReceiver.cc  -  description
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

#include <fstream>

#include "inet/transportlayer/rtp/RTPPayloadReceiver.h"

#include "inet/transportlayer/rtp/RTPInnerPacket.h"
#include "inet/transportlayer/rtp/RTPPacket.h"

namespace inet {

namespace rtp {

Define_Module(RTPPayloadReceiver);

simsignal_t RTPPayloadReceiver::_rcvdPkRtpTimestampSignal = registerSignal("rcvdPkRtpTimestamp");

RTPPayloadReceiver::~RTPPayloadReceiver()
{
    closeOutputFile();
}

void RTPPayloadReceiver::initialize()
{
    const char *fileName = par("outputFileName");
    const char *logFileName = par("outputLogFileName");
    if (strcmp(fileName, ""))
        openOutputFile(fileName);
    if (strcmp(logFileName, ""))
    {
        char logName[200];
        sprintf(logName, logFileName, getId());
        _outputLogLoss.open(logName);
    }
}

void RTPPayloadReceiver::handleMessage(cMessage *msg)
{
    RTPInnerPacket *rinp = check_and_cast<RTPInnerPacket *>(msg);
    if (rinp->getType() == RTP_INP_DATA_IN) {
        RTPPacket *packet = check_and_cast<RTPPacket *>(rinp->decapsulate());
        processPacket(packet);
        delete rinp;
    }
    else {
        //delete rinp;
        throw cRuntimeError("RTPInnerPacket of wrong type received");
    }
}

void RTPPayloadReceiver::processPacket(RTPPacket *packet)
{
    emit(_rcvdPkRtpTimestampSignal, (double)(packet->getTimeStamp()));
}

void RTPPayloadReceiver::openOutputFile(const char *fileName)
{
    _outputFileStream.open(fileName);
}

void RTPPayloadReceiver::closeOutputFile()
{
    if (_outputFileStream.is_open())
        _outputFileStream.close();
    if (_outputLogLoss.is_open())
        _outputLogLoss.close();
}

} // namespace rtp

} // namespace inet

