//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
/***************************************************************************
                          RtpPayloadReceiver.cc  -  description
                             -------------------
    begin                : Fri Nov 2 2001
    copyright            : (C) 2001 by Matthias Oppitz
    email                : Matthias.Oppitz@gmx.de
***************************************************************************/


#include "inet/transportlayer/rtp/RtpPayloadReceiver.h"

#include <fstream>

#include "inet/transportlayer/rtp/RtpInnerPacket_m.h"
#include "inet/transportlayer/rtp/RtpPacket_m.h"

namespace inet {
namespace rtp {

Define_Module(RtpPayloadReceiver);

simsignal_t RtpPayloadReceiver::rcvdPkRtpTimestampSignal = registerSignal("rcvdPkRtpTimestamp");

RtpPayloadReceiver::~RtpPayloadReceiver()
{
    closeOutputFile();
}

void RtpPayloadReceiver::initialize()
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

void RtpPayloadReceiver::handleMessage(cMessage *msg)
{
    RtpInnerPacket *rinp = check_and_cast<RtpInnerPacket *>(msg);
    if (rinp->getType() == RTP_INP_DATA_IN) {
        auto packet = check_and_cast<Packet *>(rinp->decapsulate());
        processRtpPacket(packet);
        delete rinp;
    }
    else {
        //delete rinp;
        throw cRuntimeError("RtpInnerPacket of wrong type received");
    }
}

void RtpPayloadReceiver::processRtpPacket(Packet *packet)
{
    const auto& rtpHeader = packet->peekAtFront<RtpHeader>();
    emit(rcvdPkRtpTimestampSignal, (double)(rtpHeader->getTimeStamp()));
}

void RtpPayloadReceiver::openOutputFile(const char *fileName)
{
    _outputFileStream.open(fileName);
}

void RtpPayloadReceiver::closeOutputFile()
{
    if (_outputFileStream.is_open())
        _outputFileStream.close();
    if (_outputLogLoss.is_open())
        _outputLogLoss.close();
}

} // namespace rtp
} // namespace inet

