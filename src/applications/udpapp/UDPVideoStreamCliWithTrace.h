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


#ifndef __INET_UDPVIDEOSTREAM_WITHTRACE_H
#define __INET_UDPVIDEOSTREAM_WITHTRACE_H

#include <omnetpp.h>
#include "UDPVideoStreamCli.h"
#include "UDPVideoStreamPacket_m.h"


///
/// @class UDPVideoStreamCliWithTrace
/// @brief Implementation of a video streaming client to work with
///        UDPVideoStreamSvrWithTrace
///
class INET_API UDPVideoStreamCliWithTrace : public UDPVideoStreamCli
{
  protected:
	// module parameters
	double startupDelay;
	long numTraceFrames;	///< number of frames in the trace file (needed to handle wrap around of frames by the server)
	int gopSize;	///< GOP pattern: I-to-I frame distance (N)
	int numBFrames;	///< GOP pattern: I-to-P frame distance (M)

    // statistics
	//    cOutVector eed;
	bool warmupFinished;	///< if true, start statistics gathering
	long numPacketsReceived;	///< number of packets received
	long numPacketsLost;	///< number of packets lost
    long numFramesReceived; ///< number of frames received (and correctly decoded)
    long numFramesLost; ///< number of frames lost (or discarded due to unmet dependency)

	// variables for packet and frame loss handling
//	bool isFirstPacket;	///< indicator of whether the current packet is the first one or not
	uint16_t prevSequenceNumber;	///< (16-bit RTP) sequence number of the most recently received packet
    long prevIFrameNumber;   ///< frame number of the most recently received I frame
    long prevPFrameNumber;   ///< frame number of the most recently received P frame
	long currentFrameNumber;    ///< frame number whose packets are currently being received
	long currentDisplayNumber;	///< display number of the current frame number
// 	long currentGopNumber;
	FrameType currentFrameType;
	bool currentFrameDiscard;	///< if true, all the remaining packets of the current frame are to be discarded
    bool currentFrameFinished;	///< if true, the frame has been successfully received and decoded

  protected:
    ///@name Overridden cSimpleModule functions
	//@{
	virtual void initialize();
	virtual void finish();
	virtual void handleMessage(cMessage *msg);
	//@}

  protected:
    virtual void receiveStream(UDPVideoStreamPacket *pkt);

  protected:
    // utility functions
    inline long displayNumberIFrame(long frameNumber, int numBFrames) {return (frameNumber == 0 ? 0 : frameNumber - numBFrames);}
    inline long displayNumberPFrame(long frameNumber, int numBFrames) {return (frameNumber - numBFrames);}
    inline long displayNumberBFrame(long frameNumber) {return (frameNumber - 1);}
};


#endif



