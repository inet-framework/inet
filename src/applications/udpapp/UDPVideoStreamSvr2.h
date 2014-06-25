//
// Copyright (C) 2005 Andras Varga
// Copyright (C) 2012 Alfonso Ariza
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


//
// based on the video streaming app of the similar name by Johnny Lai
//

#ifndef __INET_UDPVIDEOSTREAMSVR2_H
#define __INET_UDPVIDEOSTREAMSVR2_H


#include <vector>
#include <deque>
#include "ApplicationBase.h"
#include "UDPSocket.h"

/**
 * Stream VBR video streams to clients.
 *
 * Cooperates with UDPVideoStreamCli. UDPVideoStreamCli requests a stream
 * and UDPVideoStreamSvr starts streaming to them. Capable of handling
 * streaming to multiple clients.
 */
class INET_API UDPVideoStreamSvr2 : public ApplicationBase
{
  public:
    /**
     * Stores information on a video stream
     */
    struct VideoStreamData
    {
        cMessage *timer;          ///< self timer msg
        Address clientAddr;   ///< client address
        int clientPort;           ///< client UDP port
        long videoSize;           ///< total size of video
        long bytesLeft;           ///< bytes left to transmit
        long numPkSent;           ///< number of packets sent
        simtime_t stopTime;       ///< stop connection
        bool fileTrace;
        unsigned int traceIndex;
        simtime_t timeInit;
        VideoStreamData() { timer = NULL; clientPort = 0; videoSize = bytesLeft = 0; numPkSent = 0; }
    };

    struct VideoInfo
    {
            simtime_t timeFrame;
            uint32_t seqNum;
            char type;
            uint32_t size;
    };

  protected:
    typedef std::map<long int, VideoStreamData> VideoStreamMap;
    typedef std::deque<VideoInfo> VideoTrace;
    VideoTrace trace;

    VideoStreamData *videoBroadcastStream;
    cMessage * restartVideoBroadcast;
    int outputInterfaceBroadcast;
    bool macroPackets;
    uint64_t maxSizeMacro;
    simtime_t initTime;

    VideoStreamMap streamVector;
    UDPSocket socket;

    // module parameters
    int localPort;
    cPar *sendInterval;
    cPar *packetLen;
    cPar *videoSize;
    cPar *stopTime;

    // statistics
    unsigned int numStreams;  // number of video streams served
    unsigned long numPkSent;  // total number of packets sent
    static simsignal_t reqStreamBytesSignal;  // length of video streams served
    static simsignal_t sentPkSignal;

  protected:
    // process stream request from client
    virtual void processStreamRequest(cMessage *msg);

    // send a packet of the given video stream
    virtual void sendStreamData(cMessage *timer);

    // parse utexas video traces
    virtual void fileParser(const char *fileName);

    // begin a broadcast sequence
    virtual void broadcastVideo();
    virtual int broadcastInterface();

  public:
    UDPVideoStreamSvr2();
    virtual ~UDPVideoStreamSvr2();

  protected:
    ///@name Overridden cSimpleModule functions
    //@{
    virtual void initialize(int stage);
    virtual void finish();
    virtual void handleMessageWhenUp(cMessage* msg);
    void clearStreams();
    //@}

    //ApplicationBase:
    virtual bool handleNodeStart(IDoneCallback *doneCallback);
    virtual bool handleNodeShutdown(IDoneCallback *doneCallback);
    virtual void handleNodeCrash();
};

#endif

