//
// Copyright (C) 2012 Kyeong Soo (Joseph) Kim
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
/// @file   UDPVideoStreamSvrWithTrace2.cc
/// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
/// @date   2012-12-24
///
/// @brief  Implements UDPVideoStreamSvrWithTrace2 class.
///
/// @note
/// This file implements UDPVideoStreamSvrWithTrace2, modeling a video
/// streaming server based on trace files from ASU video trace library [1].
///
/// @par References:
/// <ol>
///	<li><a href="http://trace.eas.asu.edu/">Video trace library, Arizona State University</a>
/// </li>
/// </ol>
///


#include <cstdio>
#include <fstream>
#include "UDPVideoStreamSvrWithTrace2.h"
#include "UDPControlInfo_m.h"
#include "UDPVideoStreamPacket_m.h"


Define_Module(UDPVideoStreamSvrWithTrace2);

//------------------------------------------------------------------------------
//	Misc. functions
//------------------------------------------------------------------------------

/////
///// Tokenizes the input string.
/////
///// @param[in] str			input string
///// @param[in] tokens		string vector to include tokens
///// @param[in] delimiters	string containing delimiter characters
/////
//void Tokenize(const std::string& str, std::vector<std::string>& tokens,
//		const std::string& delimiters = " ")
//{
//	// Skip delimiters at beginning.
//	std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
//
//	// Find first "non-delimiter".
//	std::string::size_type pos = str.find_first_of(delimiters, lastPos);
//
//	while (std::string::npos != pos || std::string::npos != lastPos)
//	{
//		// Found a token, add it to the vector.
//		tokens.push_back(str.substr(lastPos, pos - lastPos));
//
//		// Skip delimiters.  Note the "not_of"
//		lastPos = str.find_first_not_of(delimiters, pos);
//
//		// Find next "non-delimiter"
//		pos = str.find_first_of(delimiters, lastPos);
//	}
//}

//inline std::ostream& operator<<(std::ostream& out,
//		const UDPVideoStreamSvrWithTrace2::VideoStreamData& d)
//{
//	out << "client=" << d.clientAddr << ":" << d.clientPort
//			<< "  seq. number=" << d.currentSequenceNumber
//			<< "  trace format=" << (d.traceFormat == ASU_TERSE ? "ASU_TERSE" : "ASU_VERBOSE")
//			<< "  number of frames=" << d.numFrames << "  frame period=" << d.framePeriod
//			<< "  current frame=" << d.currentFrame << "  frame number=" << d.frameNumber
//			<< "  frame time=" << d.frameTime << "  frame type=" << d.frameType
//			<< "  pkts sent=" << d.numPktSent << "  bytes left=" << d.bytesLeft
//			<< "  pkt interval= " << d.pktInterval << endl;
//	return out;
//}

//void UDPVideoStreamSvrWithTrace2::sendToUDP(cPacket *msg, int srcPort, const IPvXAddress& destAddr, int destPort)
//{
//    // send message to UDP, with the appropriate control info attached
//    msg->setKind(UDP_C_DATA);
//
//    UDPControlInfo *ctrl = new UDPControlInfo();
//    ctrl->setSrcPort(srcPort);
//    ctrl->setDestAddr(destAddr);
//    ctrl->setDestPort(destPort);
//    msg->setControlInfo(ctrl);
//
//    EV << "Sending packet: ";
//#ifndef NDEBUG
//    printPacket(msg);
//#endif
//
//    send(msg, "udpOut");
//}

//UDPVideoStreamSvrWithTrace2::UDPVideoStreamSvrWithTrace2()
//{
//}
//
//UDPVideoStreamSvrWithTrace2::~UDPVideoStreamSvrWithTrace2()
//{
//	for (unsigned int i = 0; i < streamVector.size(); i++)
//	{
//		cancelAndDelete(streamVector[i]->frameStartMsg);
//		cancelAndDelete(streamVector[i]->packetTxMsg);
//		delete streamVector[i];
//	}
//}

void UDPVideoStreamSvrWithTrace2::initialize()
{
    UDPVideoStreamSvrWithTrace::initialize();

    clockFrequency = par("clockFrequency").doubleValue();
}

//void UDPVideoStreamSvrWithTrace::finish()
//{
//	recordScalar("streams served", numStreams);
//	recordScalar("packets sent", numPktSent);
//}

//void UDPVideoStreamSvrWithTrace::handleMessage(cMessage *msg)
//{
//	if (msg->isSelfMessage())
//	{
//		if (msg->getKind() == FRAME_START)
//		{
//			readFrameData(msg);
//		}
//		else
//		{
//			sendStreamData(msg);
//		}
//	}
//	else
//	{
//		// start streaming
//		processStreamRequest(msg);
//	}
//}

//void UDPVideoStreamSvrWithTrace::processStreamRequest(cMessage *msg)
//{
//	// register video stream...
//	UDPControlInfo *ctrl = check_and_cast<UDPControlInfo *> (
//			msg->getControlInfo());
//
//	VideoStreamData *d = new VideoStreamData;
//	d->clientAddr = ctrl->getSrcAddr();
//	d->clientPort = ctrl->getSrcPort();
//	d->currentSequenceNumber = intuniform(0, 65535);	///< made random according to RFC 3550
//	d->numPktSent = 0;
//	d->numFrames = numFrames;
//	d->framePeriod = framePeriod;
////	d->currentFrame = intuniform(1, numFrames); ///< start frame is randomly selected; here we assume sizeof(int) = sizeof(long)
//	d->currentFrame = intuniform(0, numFrames-1); ///< start frame is randomly selected; here we assume sizeof(int) = sizeof(long)
//	streamVector.push_back(d);
//
//	// initialize self messages
//	d->frameStartMsg = new cMessage("Start of Frame", FRAME_START);
//	d->frameStartMsg->setContextPointer(d);
//	d->packetTxMsg = new cMessage("Packet Transmission", PACKET_TX);
//	d->packetTxMsg->setContextPointer(d);
//
//	// read frame data from the vector and trigger packet transmission
//	readFrameData(d->frameStartMsg);
//
//	numStreams++;
//    delete msg;
//}

//void UDPVideoStreamSvrWithTrace::readFrameData(cMessage *frameTimer)
//{
//	VideoStreamData *d = (VideoStreamData *) frameTimer->getContextPointer();
//
////	d->currentFrame = (d->currentFrame < numFrames) ? d->currentFrame + 1 : 1; ///> wrap around to the first frame if it reaches the last one
//	d->currentFrame = (d->currentFrame + 1) % numFrames; ///> wrap around to the first frame if it reaches the last one
//	d->frameNumber = frameNumberVector[d->currentFrame];
//	d->frameTime = frameTimeVector[d->currentFrame];
//	d->frameType = frameTypeVector[d->currentFrame];
//	d->frameSize = frameSizeVector[d->currentFrame];
//	d->bytesLeft = d->frameSize;
//	d->pktInterval = d->framePeriod / ceil(d->bytesLeft / double(maxPayloadSize));	///> spread out packet transmissions over the frame period
//
//	// schedule next frame start
//	scheduleAt(simTime() + framePeriod, frameTimer);
//
//	// start packet transmission right away
//	sendStreamData(d->packetTxMsg);
//}

void UDPVideoStreamSvrWithTrace2::sendStreamData(cMessage *pktTimer)
{
	VideoStreamData *d = (VideoStreamData *) pktTimer->getContextPointer();

	// generate and send a packet
	UDPVideoStreamPacket *pkt = new UDPVideoStreamPacket("UDPVideoStreamPacket");
	long payloadSize = (d->bytesLeft >= maxPayloadSize) ? maxPayloadSize : d->bytesLeft;
	pkt->setByteLength(payloadSize + appOverhead);
	pkt->setSequenceNumber(d->currentSequenceNumber);	///< 16-bit RTP sequence number
	pkt->setTimestamp(uint32_t(uint64_t(clockFrequency*simTime().dbl())%0x100000000LL));    ///< 32-bit RTP timestamp (wrap-arounded)
//    pkt->setTimestamp(uint32_t((uint64_t(clockFrequency)*simTime().raw()/simTime().getScale())%0x100000000LL));    ///< 32-bit RTP timestamp (wrap-arounded)
	pkt->setFragmentStart(d->bytesLeft == d->frameSize ? true : false);	///< in FU header in RTP payload
	pkt->setFragmentEnd(d->bytesLeft == payloadSize ? true : false);	///< in FU header in RTP payload
	pkt->setFrameNumber(d->frameNumber);	///< non-RTP field
	pkt->setFrameTime(d->frameTime);	///< non-RTP field
	pkt->setFrameType(d->frameType);	///> non-RTP field
	sendToUDP(pkt, serverPort, d->clientAddr, d->clientPort);

	// update the session VideoStreamData and global statistics
	d->bytesLeft -= payloadSize;
	d->numPktSent++;
	d->currentSequenceNumber = (d->currentSequenceNumber  + 1) % 0x10000L;  ///> wrap around to zero if it reaches the maximum value (65535)
	numPktSent++;

	// reschedule timer if there are bytes left to send
	if (d->bytesLeft != 0)
	{
		scheduleAt(simTime() + d->pktInterval, pktTimer);
	}
}
