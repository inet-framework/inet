///
/// @file   UDPVideoStreamCliWithTrace.h
/// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
/// @date   2010-04-02
///
/// @brief  Declares UDPVideoStreamCliWithTrace class, modeling a video
///         streaming client based on trace files from ASU video trace
///         library [1].
///
/// @par References:
/// <ol>
///	<li><a href="http://trace.eas.asu.edu/">Video trace library, Arizona State University</a>
/// </li>
/// </ol>
///
/// @remarks Copyright (C) 2010 Kyeong Soo (Joseph) Kim. All rights reserved.
///
/// @remarks This software is written and distributed under the GNU General
///          Public License Version 2 (http://www.gnu.org/licenses/gpl-2.0.html).
///          You must not remove this notice, or any other, from this software.
///


#ifndef UDPVIDEOSTREAMCLI_WITH_TRACE_H
#define UDPVIDEOSTREAMCLI_WITH_TRACE_H

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
	bool warmupFinished;	///< if true, start statistics gathering
	long numPacketsReceived;	///< number of packets received
	long numPacketsLost;	///< number of packets lost
    long numFramesReceived;	///< number of frames received and correctly decoded
    long numFramesDiscarded;	///< number of frames discarded due to packet loss or failed dependency

	// variables for packet and frame loss handling
	uint16_t prevSequenceNumber;	///< (16-bit RTP) sequence number of the most recently received packet
    long prevIFrameNumber;   ///< frame number of the most recently received I frame
    long prevPFrameNumber;   ///< frame number of the most recently received P frame
	long currentFrameNumber;    ///< frame number (display order) of the frame under processing
	long currentEncodingNumber;	///< encoding number (transmission order) of the frame under processing
	FrameType currentFrameType;	/// type (I, IDR, P, or B) of the frame under processing
	bool currentFrameDiscard;	///< if true, all the remaining packets of the current frame are to be discarded
    bool currentFrameFinished;	///< if true, the frame has been successfully received and decoded

  protected:
	virtual void initialize();
	virtual void finish();
	virtual void handleMessage(cMessage *msg);

  protected:
    virtual void receiveStream(UDPVideoStreamPacket *pkt);

  protected:
    // utility functions
    long frameEncodingNumber(long frameNumber, int numBFrames, FrameType frameType);
};


#endif	// UDPVIDEOSTREAMCLI_WITH_TRACE_H
