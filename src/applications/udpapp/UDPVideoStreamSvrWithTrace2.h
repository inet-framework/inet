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


#ifndef UDPVIDEOSTREAMSVR_WITH_TRACE2_H
#define UDPVIDEOSTREAMSVR_WITH_TRACE2_H

#include "UDPVideoStreamSvrWithTrace.h"


///
/// @class UDPVideoStreamSvrWithTrace2
/// @brief Implementation of a video streaming server based on trace files
///        from ASU video trace library [1].
///
/// @par References:
/// <ol>
///	<li><a href="http://trace.eas.asu.edu/">Video trace library, Arizona State University</a>
/// </li>
/// </ol>
///
class INET_API UDPVideoStreamSvrWithTrace2 : public UDPVideoStreamSvrWithTrace
{
  public:
    /**
     * Stores information on a video stream
     */
//    struct VideoStreamData
//    {
//    	// client info.
//        IPvXAddress clientAddr;	///< client address
//        int clientPort;			///< client UDP port

        // packet generation
//        uint16_t currentSequenceNumber;	///< current 16-bit RTP sequence number

//        // variable for a video trace
//        TraceFormat traceFormat;	///< file format of trace file
//        long numFrames;			///< total number of frames for a video trace
//        double framePeriod;		///< frame period for a video trace
//        long currentFrame;		///< frame index (starting from 0) to read from the trace (will be wrapped around);
//        long frameNumber;		///< frame number (display order) of the current frame
//        double frameTime;		///< cumulative display time of the current frame in millisecond
//        FrameType frameType;	///< type of the current frame
//        long frameSize;			///< size of the current frame in byte
//        long bytesLeft;			///< bytes left to transmit in the current frame
//        double pktInterval;		///< interval between consecutive packet transmissions in a given frame
//
//        // statistics
//        long numPktSent;           ///< number of packets sent
//
//        // self messages for timers
//        cMessage *frameStartMsg;  ///< start of each frame
//        cMessage *packetTxMsg;    ///< start of each packet transmission
//    };
//    typedef std::vector<VideoStreamData *> VideoStreamVector;

  protected:
//    VideoStreamVector streamVector;

    // module parameters
//    int serverPort;
//    int appOverhead;
//    int maxPayloadSize;
    double clockFrequency;

//    // variables for a trace file
//    TraceFormat traceFormat;	///< file format of trace file
//    long numFrames;	///< total number of frames in a trace file
//    double framePeriod;
//    LongVector frameNumberVector;	///< vector of frame numbers (display order) (only for verbose trace)
//    DoubleVector frameTimeVector;	///< vector of cumulative frame display times (only for verbose trace)
//    FrameTypeVector frameTypeVector;	///< vector of frame types (I, P, B, and IDR (H.264); only for verbose trace)
//    LongVector frameSizeVector;	///< vector of frame sizes [byte]

//    // statistics
//    unsigned int numStreams;  // number of video streams served
//    unsigned long numPktSent;  // total number of packets sent

  protected:
//    // process stream request from client
//    virtual void processStreamRequest(cMessage *msg);
//
//    // read new frame data and update relevant variables
//    virtual void readFrameData(cMessage *frameTimer);

    // send a packet of the given video stream
    virtual void sendStreamData(cMessage *pktTimer);

  public:
//    UDPVideoStreamSvrWithTrace();
//    virtual ~UDPVideoStreamSvrWithTrace();

  protected:
    ///@name Overidden cSimpleModule functions
    //@{
    virtual void initialize();
//    virtual void finish();
//    virtual void handleMessage(cMessage* msg);
    //@}

  protected:
//    /// redefine to optimize the performance
//    virtual void sendToUDP(cPacket *msg, int srcPort, const IPvXAddress& destAddr, int destPort);
};


#endif	// UDPVIDEOSTREAMSVR_WITH_TRACE_H
