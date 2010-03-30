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
/// This file implements UDPVideoStreamSvrWithTrace, modelling a video
/// streaming server based on trace files in ASU <i>terse</i> format [1].
/// It is based on the video streaming interface packages originally
/// developed by Signorin Luca (luca.signorin@inwind.it), University of
/// Ferrara, Italy, for OMNeT++ 2.2 and later update by Dr. Fitzek for
/// OMNeT++ 3
///
/// @par References:
/// <ol>
///	<li><a href="http://trace.eas.asu.edu/">Video trace library, Arizona State University</a>
/// </li>
/// </ol>
///


//#include <iostream>
#include <fstream>
#include "UDPVideoStreamSvrWithTrace.h"
#include "UDPControlInfo_m.h"
#include "UDPVideoStreamPacket_m.h"


Define_Module(UDPVideoStreamSvrWithTrace);

inline std::ostream& operator<<(std::ostream& out,
		const UDPVideoStreamSvrWithTrace::VideoStreamData& d)
{
	out << "client=" << d.clientAddr << ":" << d.clientPort
			<< "  pkts sent=" << d.numPktSent << " current frame=" << d.currentFrame
			<< "  bytes left=" << d.bytesLeft << " pkt interval= " << d.pktInterval << endl;
	return out;
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
	framePeriod = 1.0 / par("fps").longValue();
	numFrames = 0;

	// read frame size data from the trace file into 'frameSizeVector'
	const char *fileName = par("traceFile").stringValue();
	std::ifstream fin(fileName);
	if (fin.is_open())
	{
		long frameSize;
		double psnr_y;
		while (!fin.eof())
		{
			fin >> frameSize >> psnr_y;
			frameSizeVector.push_back(frameSize / 8); ///< in byte
			numFrames++;
		}
		fin.close();
	}
	else
	{
		error("%s: Unable to open the video trace file `%s'",
				getFullPath().c_str(), fileName);
	}

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
			readFrameSize(msg);
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
	d->framePeriod = framePeriod;
	d->currentFrame = intuniform(1, numFrames); ///< start frame is randomly selected; here we assume sizeof(int) = sizeof(long)
	streamVector.push_back(d);

	// initialize self messages
	d->frameStartMsg = new cMessage("Start of Frame", FRAME_START);
	d->frameStartMsg->setContextPointer(d);
	d->packetTxMsg = new cMessage("Packet Transmission", PACKET_TX);
	d->packetTxMsg->setContextPointer(d);

	// read a frame size from the vector and trigger packet transmission
	readFrameSize(d->frameStartMsg);

	numStreams++;
}

void UDPVideoStreamSvrWithTrace::readFrameSize(cMessage *frameTimer)
{
	VideoStreamData *d = (VideoStreamData *) frameTimer->getContextPointer();

	d->bytesLeft = frameSizeVector[d->currentFrame];
	d->pktInterval = d->framePeriod / ceil(d->bytesLeft / double(maxPayloadSize));
	d->currentFrame = (d->currentFrame < numFrames) ? d->currentFrame + 1 : 1; ///> wrap around to the first frame if it reaches the last one

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
	pkt->setSequenceNumber(d->currentSequenceNumber);
	// TODO: Add time stamp and other field processing here
	sendToUDP(pkt, serverPort, d->clientAddr, d->clientPort);

	d->bytesLeft -= payloadSize;
	d->numPktSent++;
	d->currentSequenceNumber = (d->currentSequenceNumber < 65535) ? d->currentSequenceNumber + 1 : 0;	///> wrap around to zero if it reaches the maximum value (65535)
	numPktSent++;

	// reschedule timer if there's bytes left to send
	if (d->bytesLeft != 0)
	{
		scheduleAt(simTime() + d->pktInterval, pktTimer);
	}
	//	else
	//	{
	//		delete pktTimer;
	//		// TBD find VideoStreamData in streamVector and delete it
	//	}
}
