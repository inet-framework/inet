//
// Copyright (C) 2010 Kyeong Soo (Joseph) Kim
// Copyright (C) 2005 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

///
/// @file   UDPVideoStreamSvrWithTrace.cc
/// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
/// @date   2010-03-26
///
/// @brief  Implements UDPVideoStreamSvrWithTrace class.
///
/// @note
/// This file implements UDPVideoStreamSvrWithTrace, modeling a video
/// streaming server based on trace files from ASU video trace library [1].
///
/// @par References:
/// <ol>
///	<li><a href="http://trace.eas.asu.edu/">Video trace library, Arizona State University</a>
/// </li>
/// </ol>
///


//#include <iostream>
#include <fstream>
#include <stdio.h>
#include "UDPVideoStreamSvrWithTrace.h"
#include "UDPControlInfo_m.h"
#include "UDPVideoStreamPacket_m.h"


Define_Module(UDPVideoStreamSvrWithTrace);

//------------------------------------------------------------------------------
//	Misc. functions
//------------------------------------------------------------------------------

///
/// Tokenizes the input string.
///
/// @param[in] str			input string
/// @param[in] tokens		string vector to include tokens
/// @param[in] delimiters	string containing delimiter characters
///
void Tokenize(const std::string& str, std::vector<std::string>& tokens,
		const std::string& delimiters = " ")
{
	// Skip delimiters at beginning.
	std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);

	// Find first "non-delimiter".
	std::string::size_type pos = str.find_first_of(delimiters, lastPos);

	while (std::string::npos != pos || std::string::npos != lastPos)
	{
		// Found a token, add it to the vector.
		tokens.push_back(str.substr(lastPos, pos - lastPos));

		// Skip delimiters.  Note the "not_of"
		lastPos = str.find_first_not_of(delimiters, pos);

		// Find next "non-delimiter"
		pos = str.find_first_of(delimiters, lastPos);
	}
}

inline std::ostream& operator<<(std::ostream& out,
		const UDPVideoStreamSvrWithTrace::VideoStreamData& d)
{
	out << "client=" << d.clientAddr << ":" << d.clientPort
			<< "  seq. number=" << d.currentSequenceNumber
			<< "  trace format=" << (d.traceFormat == ASU_TERSE ? "ASU_TERSE" : "ASU_VERBOSE")
			<< "  number of frames=" << d.numFrames << "  frame period=" << d.framePeriod
			<< "  current frame=" << d.currentFrame << "  frame number=" << d.frameNumber
			<< "  frame time=" << d.frameTime << "  frame type=" << d.frameType
			<< "  pkts sent=" << d.numPktSent << "  bytes left=" << d.bytesLeft
			<< "  pkt interval= " << d.pktInterval << endl;
	return out;
}

void UDPVideoStreamSvrWithTrace::sendToUDP(cPacket *msg, int srcPort, const IPvXAddress& destAddr, int destPort)
{
    // send message to UDP, with the appropriate control info attached
    msg->setKind(UDP_C_DATA);

    UDPControlInfo *ctrl = new UDPControlInfo();
    ctrl->setSrcPort(srcPort);
    ctrl->setDestAddr(destAddr);
    ctrl->setDestPort(destPort);
    msg->setControlInfo(ctrl);

    EV << "Sending packet: ";
#ifndef NDEBUG
    printPacket(msg);
#endif

    send(msg, "udpOut");
}

UDPVideoStreamSvrWithTrace::UDPVideoStreamSvrWithTrace()
{
}

UDPVideoStreamSvrWithTrace::~UDPVideoStreamSvrWithTrace()
{
	for (unsigned int i = 0; i < streamVector.size(); i++)
	{
		cancelAndDelete(streamVector[i]->frameStartMsg);
		cancelAndDelete(streamVector[i]->packetTxMsg);
		delete streamVector[i];
	}
}

void UDPVideoStreamSvrWithTrace::initialize()
{
	serverPort = par("serverPort");
	appOverhead = par("appOverhead").longValue();
	maxPayloadSize = par("maxPayloadSize").longValue();
    frameSpreading = par("frameSpreading").boolValue();
	framePeriod = 1.0 / par("fps").longValue();
	numFrames = 0;

	// read frame data from the trace file into corresponding vectors (e.g., frameSizeVector)
	const char *fileName = par("traceFile").stringValue();
	std::ifstream fin(fileName);
	if (fin.is_open())
	{
		int currentPosition;
		std::string line;

		// skip comments
		do {
			currentPosition = fin.tellg();	// save the current position
			std::getline(fin, line);
		} while (line[0] == '#');

		// file format (i.e., 'terse' or 'verbose') detection
		StringVector tokens;
		Tokenize(line, tokens, " \t");
		traceFormat = (tokens.size() > 2) ? ASU_VERBOSE : ASU_TERSE;

		fin.seekg (currentPosition);	// go back to the first non-comment line
		long frameNumber = 0;
		double frameTime = 0.0;
		std::string frameTypeString("");
		FrameType frameType =  I;
		long frameSize;
		double psnr_y = 0.0;
		double psnr_u = 0.0;
		double psnr_v = 0.0;

		if (traceFormat == ASU_TERSE)
		{
			// file format is ASU_TERSE
			while (fin >> frameSize >> psnr_y)	///< never use "!fin.eof()" to check the EOF!
			{
				frameNumberVector.push_back(frameNumber);
				frameTimeVector.push_back(frameTime);
				frameTypeVector.push_back(frameType);
				frameSizeVector.push_back(frameSize / 8); ///< in byte

				// manually update the following fields
				frameNumber++;
				frameTime += framePeriod;

				numFrames++;
			}
		}
		else
		{
			// file format is ASU_VERBOSE
			while (fin >> frameNumber >> frameTime >> frameTypeString >> frameSize
					>> psnr_y >> psnr_u >> psnr_v)
			{
				frameNumberVector.push_back(frameNumber);
				frameTimeVector.push_back(frameTime);
				if (frameTypeString.compare("I") == 0)
					frameType = I;
				else if (frameTypeString.compare("IDR") == 0)
					frameType = IDR;
				else if (frameTypeString.compare("P") == 0)
					frameType = P;
				else
					frameType = B;
				frameTypeVector.push_back(frameType);
				frameSizeVector.push_back(frameSize / 8); ///< in byte
				numFrames++;
			}
		}

		fin.close();
	}	// end of if(fin.is_open())
	else
	{
		error("%s: Unable to open the video trace file `%s'",
				getFullPath().c_str(), fileName);
	}

	// initialize statistics
	numStreams = 0;
	numPktSent = 0;

	WATCH_PTRVECTOR(streamVector);

	bindToPort(serverPort);
}

void UDPVideoStreamSvrWithTrace::finish()
{
	recordScalar("streams served", numStreams);
	recordScalar("packets sent", numPktSent);
}

void UDPVideoStreamSvrWithTrace::handleMessage(cMessage *msg)
{
	if (msg->isSelfMessage())
	{
		if (msg->getKind() == FRAME_START)
		{
			readFrameData(msg);
		}
		else
		{
			sendStreamData(msg);
		}
	}
	else
	{
		// start streaming
		processStreamRequest(msg);
	}
}

void UDPVideoStreamSvrWithTrace::processStreamRequest(cMessage *msg)
{
	// register video stream...
	UDPControlInfo *ctrl = check_and_cast<UDPControlInfo *> (
			msg->getControlInfo());

	VideoStreamData *d = new VideoStreamData;
	d->clientAddr = ctrl->getSrcAddr();
	d->clientPort = ctrl->getSrcPort();
	d->currentSequenceNumber = intuniform(0, 65535);	///< made random according to RFC 3550
	d->numPktSent = 0;
	d->numFrames = numFrames;
    d->numFramesSent = 0;
	d->framePeriod = framePeriod;
	d->currentFrame = intuniform(0, d->numFrames-1); ///< start frame is randomly selected; here we assume sizeof(int) = sizeof(long)
	streamVector.push_back(d);

	// initialize self messages
	d->frameStartMsg = new cMessage("Start of Frame", FRAME_START);
	d->frameStartMsg->setContextPointer(d);
	d->packetTxMsg = new cMessage("Packet Transmission", PACKET_TX);
	d->packetTxMsg->setContextPointer(d);

	// read frame data from the vector and trigger packet transmission
	readFrameData(d->frameStartMsg);

	numStreams++;
    delete msg;
}

void UDPVideoStreamSvrWithTrace::readFrameData(cMessage *frameTimer)
{
	VideoStreamData *d = (VideoStreamData *) frameTimer->getContextPointer();

    if (d->numFramesSent == d->numFrames)
    {   // sent the whole frames in the trace file; reset the frame counter
        d->numFramesSent = 0;
        d->currentFrame = intuniform(0, d->numFrames-1); ///< start frame is randomly selected; here we assume sizeof(int) = sizeof(long)
    }
    else {
        d->numFramesSent++;
    }
	d->currentFrame = (d->currentFrame + 1) % d->numFrames; ///> wrap around to the first frame if it reaches the last one
	d->frameNumber = frameNumberVector[d->currentFrame];
	d->frameTime = frameTimeVector[d->currentFrame];
	d->frameType = frameTypeVector[d->currentFrame];
	d->frameSize = frameSizeVector[d->currentFrame];
	d->bytesLeft = d->frameSize;
    if (frameSpreading)
    {
        d->pktInterval = d->framePeriod / ceil(d->bytesLeft / double(maxPayloadSize));	///> spread out packet transmissions over a frame period
    }
    else
    {
        d->pktInterval = 0.0;
    }

	// schedule next frame start
	scheduleAt(simTime() + framePeriod, frameTimer);

	// start packet transmission right away
	sendStreamData(d->packetTxMsg);
}

void UDPVideoStreamSvrWithTrace::sendStreamData(cMessage *pktTimer)
{
	VideoStreamData *d = (VideoStreamData *) pktTimer->getContextPointer();

	// generate and send a packet
	UDPVideoStreamPacket *pkt = new UDPVideoStreamPacket("UDPVideoStreamPacket");
	long payloadSize = (d->bytesLeft >= maxPayloadSize) ? maxPayloadSize : d->bytesLeft;
	pkt->setByteLength(payloadSize + appOverhead);
	pkt->setMarker(d->bytesLeft == payloadSize ? true : false); ///< indicator for the last packet of a frame
	pkt->setSequenceNumber(d->currentSequenceNumber);	///< in RTP header
	pkt->setFragmentStart(d->bytesLeft == d->frameSize ? true : false);	///< in FU header in RTP payload
	pkt->setFragmentEnd(pkt->getMarker);    ///< in FU header in RTP payload
	pkt->setFrameNumber(d->frameNumber);	///< non-RTP field
	pkt->setFrameTime(d->frameTime);	///< non-RTP field
	pkt->setFrameType(d->frameType);	///> non-RTP field
	sendToUDP(pkt, serverPort, d->clientAddr, d->clientPort);

	// update the session VideoStreamData and global statistics
	d->bytesLeft -= payloadSize;
	d->numPktSent++;
	d->currentSequenceNumber = (d->currentSequenceNumber  + 1) % 65536;	///> wrap around to zero if it reaches the maximum value (65535)
	numPktSent++;

	// reschedule timer if there are bytes left to send
	if (d->bytesLeft != 0)
	{
		scheduleAt(simTime() + d->pktInterval, pktTimer);
	}
}
