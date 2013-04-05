//
// Copyright (C) 2013 Kyeong Soo (Joseph) Kim
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
/// @file   UDPVideoStreamSvrWithTrace3.cc
/// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
/// @date   2013-03-29
///
/// @brief  Implements UDPVideoStreamSvrWithTrace3 class.
///
/// @note
/// This file implements UDPVideoStreamSvrWithTrace3, modeling a video
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
#include "UDPVideoStreamSvrWithTrace3.h"
#include "UDPControlInfo_m.h"
#include "UDPVideoStreamPacket_m.h"


Define_Module(UDPVideoStreamSvrWithTrace3);


void UDPVideoStreamSvrWithTrace3::initialize()
{
    UDPVideoStreamSvrWithTrace2::initialize();

    N = par("N").longValue();
    timingPktPeriod = par("timingPktPeriod").longValue();
    f_r = clockFrequency / N;   // reference clock frequency
    pktInterval = 1.0 / f_r;

}

void UDPVideoStreamSvrWithTrace3::sendStreamData(cMessage *pktTimer)
{   // note that the current implementation is for N = 1 only
    // TODO: Extend it to the general case of N > 1

	VideoStreamData *d = (VideoStreamData *) pktTimer->getContextPointer();

	// generate and send a packet
	UDPVideoStreamPacket *pkt = new UDPVideoStreamPacket("UDPVideoStreamPacket");
	long payloadSize = maxPayloadSize;  ///< every RTP packet has the same size in JTS SCFR algorithm
	// long payloadSize = (d->bytesLeft >= maxPayloadSize) ? maxPayloadSize : d->bytesLeft;
	pkt->setByteLength(payloadSize + appOverhead);
	pkt->setMarker(d->bytesLeft <= maxPayloadSize ? true : false);  ///< indicator for the last packet of a frame
	pkt->setSequenceNumber(d->currentSequenceNumber);	///< 16-bit RTP sequence number
	pkt->setTimestamp(uint32_t(uint64_t(f_r*simTime().dbl())%0x100000000LL));   ///< 32-bit RTP timestamp (wrap-arounded)
//    pkt->setTimestamp(uint32_t((uint64_t(clockFrequency)*simTime().raw()/simTime().getScale())%0x100000000LL));    ///< 32-bit RTP timestamp (wrap-arounded)
	pkt->setFragmentStart(d->bytesLeft == d->frameSize ? true : false);	///< in FU header in RTP payload
	pkt->setFragmentEnd(pkt->getMarker());    ///< in FU header in RTP payload
	pkt->setFrameNumber(d->frameNumber);	///< non-RTP field
	pkt->setFrameTime(d->frameTime);	///< non-RTP field
	pkt->setFrameType(d->frameType);	///> non-RTP field
	sendToUDP(pkt, serverPort, d->clientAddr, d->clientPort);

	// update the session VideoStreamData and global statistics
    d->bytesLeft = (d->bytesLeft > payloadSize) ? (d->bytesLeft - payloadSize) : 0; // take into account the case when d->bytesLeft < payloadSize
	d->numPktSent++;
	d->currentSequenceNumber = (d->currentSequenceNumber  + 1) % 0x10000L;  ///> wrap around to zero if it reaches the maximum value (65535)
	numPktSent++;

	// reschedule timer if there are bytes left to send
	if (d->bytesLeft != 0)
	{
		// scheduleAt(simTime() + d->pktInterval, pktTimer);
		scheduleAt(simTime() + pktInterval, pktTimer); // packet interval based on the reference clock frequency
	}
}
