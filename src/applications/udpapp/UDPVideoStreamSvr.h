//
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


//
// based on the video streaming app of the similar name by Johnny Lai
//

#ifndef __INET_UDPVIDEOSTREAMSVR_H
#define __INET_UDPVIDEOSTREAMSVR_H


#include <map>

#include "INETDefs.h"

#include "AppBase.h"
#include "UDPSocket.h"


/**
 * Stream VBR video streams to clients.
 *
 * Cooperates with UDPVideoStreamCli. UDPVideoStreamCli requests a stream
 * and UDPVideoStreamSvr starts streaming to them. Capable of handling
 * streaming to multiple clients.
 */
class INET_API UDPVideoStreamSvr : public AppBase
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
        VideoStreamData() { timer = NULL; clientPort = 0; videoSize = bytesLeft = 0; numPkSent = 0; }
    };

  protected:
    typedef std::map<long int, VideoStreamData> VideoStreamMap;
    VideoStreamMap streams;
    UDPSocket socket;

    // module parameters
    int localPort;
    cPar *sendInterval;
    cPar *packetLen;
    cPar *videoSize;

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

  public:
    UDPVideoStreamSvr();
    virtual ~UDPVideoStreamSvr();

  protected:
    ///@name Overridden InetSimpleModule functions
    //@{
    virtual int numInitStages() const { return NUM_INIT_STAGES; }
    virtual void initialize(int stage);
    virtual void finish();
    virtual void handleMessageWhenUp(cMessage* msg);
    //@}

    virtual void clearStreams();

    //AppBase:
    virtual bool startApp(IDoneCallback *doneCallback);
    virtual bool stopApp(IDoneCallback *doneCallback);
    virtual bool crashApp(IDoneCallback *doneCallback);
};

#endif

