//
// Copyright (C) 2005 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//


//
// based on the video streaming app of the similar name by Johnny Lai
//

#ifndef UDPVIDEOSTREAMSVR_H
#define UDPVIDEOSTREAMSVR_H

#include <vector>
#include <omnetpp.h>
#include "UDPAppBase.h"


/**
 * Stream VBR video streams to clients.
 *
 * Cooperates with UDPVideoStreamCli. UDPVideoStreamCli requests a stream
 * and UDPVideoStreamSvr starts streaming to them. Capable of handling
 * streaming to multiple clients.
 */
class INET_API UDPVideoStreamSvr : public UDPAppBase
{
  public:
    /**
     * Stores information on a video stream
     */
    struct VideoStreamData
    {
        IPvXAddress clientAddr;   ///< client address
        int clientPort;           ///< client UDP port
        unsigned long videoSize;  ///< total size of video
        unsigned long bytesLeft;  ///< bytes left to transmit
        unsigned long numPkSent;  ///< number of packets sent
    };

  protected:
    typedef std::vector<VideoStreamData *> VideoStreamVector;
    VideoStreamVector streamVector;

    // module parameters
    int serverPort;
    cPar *waitInterval;
    cPar *packetLen;
    cPar *videoSize;

    // statistics
    unsigned int numStreams;  // number of video streams served
    unsigned long numPkSent;  // total number of packets sent

  protected:
    // process stream request from client
    void processStreamRequest(cMessage *msg);

    // send a packet of the given video stream
    void sendStreamData(cMessage *timer);

  public:
    UDPVideoStreamSvr();
    virtual ~UDPVideoStreamSvr();

  protected:
    ///@name Overidden cSimpleModule functions
    //@{
    virtual void initialize();
    virtual void finish();
    virtual void handleMessage(cMessage* msg);
    //@}
};


#endif


