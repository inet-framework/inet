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


#ifndef UDPVIDEOSTREAMSVR_WITHTRACE_H
#define UDPVIDEOSTREAMSVR_WITHTRACE_H

#include <vector>
#include <omnetpp.h>
#include "UDPAppBase.h"


///
/// @class UDPVideoStreamSvrWithTrace
/// @brief Implementation of a video streaming server based on the
/// interface packages originally developed by Signorin Luca
/// (luca.signorin@inwind.it), University of Ferrara, Italy,
/// for OMNeT++ 2.2 and later update by Dr. Fitzek for OMNeT++ 3
///
class INET_API UDPVideoStreamSvrWithTrace : public UDPAppBase
{
  public:
    /**
     * Stores information on a video stream
     */
    struct VideoStreamData
    {
    	// client info.
        IPvXAddress clientAddr;	///< client address
        int clientPort;			///< client UDP port

        // variable for a video trace
//        long videoSize;           ///< total size of video
        long numFrames;			///< total number of frames for a video trace
        double framePeriod;		///< frame period for a video trace
        long currentFrame;		///< current frame number (will be wrapped around)
        long bytesLeft;			///< bytes left to transmit in the current frame
        double pktInterval;		///< interval between consecutive packet transmissions in a given frame

        // statistics
        long numPktSent;           ///< number of packets sent

        // self messages for timers
        cMessage *frameStartMsg;  ///< start of each frame
        cMessage *packetTxMsg;    ///< start of each packet transmission
    };

    /// kind values for self messages
    enum MessageKind
	{
		FRAME_START	= 100,
		PACKET_TX	= 200
	};

  protected:
    typedef std::vector<VideoStreamData *> VideoStreamVector;
    VideoStreamVector streamVector;
    typedef std::vector<long> LongVector;
    LongVector frameSizeVector;	///< vector of frame size [byte]

    // module parameters
    int serverPort;
    int appOverhead;
    int maxPayloadSize;
//    cPar *waitInterval;
//    cPar *packetLen;
//    cPar *videoSize;

    // variables for a trace file
    long numFrames;	///< total number of frames in a trace file
    double framePeriod;

    // statistics
    unsigned int numStreams;  // number of video streams served
    unsigned long numPktSent;  // total number of packets sent

  protected:
    // process stream request from client
    virtual void processStreamRequest(cMessage *msg);

    // read a new frame size and update relevant variables
    virtual void readFrameSize(cMessage *frameTimer);

    // send a packet of the given video stream
    virtual void sendStreamData(cMessage *pktTimer);

  public:
    UDPVideoStreamSvrWithTrace();
    virtual ~UDPVideoStreamSvrWithTrace();

  protected:
    ///@name Overidden cSimpleModule functions
    //@{
    virtual void initialize();
    virtual void finish();
    virtual void handleMessage(cMessage* msg);
    //@}
};


#endif	// UDPVIDEOSTREAMSVR_WITHTRACE_H
