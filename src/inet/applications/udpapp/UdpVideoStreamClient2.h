//
// Copyright (C) 2005 Andras Varga
// Copyright (C) 2015 A. Ariza (Malaga University)
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

#ifndef __INET_UDPVIDEOSTREAMCLIENT2_H
#define __INET_UDPVIDEOSTREAMCLIENT2_H

#include "inet/common/INETDefs.h"

#include "inet/applications/base/ApplicationBase.h"
#include "inet/common/packet/Packet.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"
/**
 * A "Realtime" VideoStream client application.
 *
 * Basic video stream application. Clients connect to server and get a stream of
 * video back.
 */

namespace inet {

class INET_API UdpVideoStreamClient2 :  public ApplicationBase, public UdpSocket::ICallback
{
  protected:
    UdpSocket socket;
    bool socketOpened;

    // statistics
    static simsignal_t rcvdPkSignal;
    cMessage * reintentTimer;
    cMessage * timeOutMsg;
    cMessage * selfMsg;

    double timeOut;
    double limitDelay;
    int numRecPackets;

    uint32_t numPframes;
    uint32_t numIframes;
    uint32_t numBframes;
    uint64_t totalBytesI;
    uint64_t totalBytesP;
    uint64_t totalBytesB;
    int64_t lastSeqNum;

    bool recieved;

  protected:
    ///@name Overridden cSimpleModule functions
    //@{
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void finish() override;
    virtual void handleMessageWhenUp(cMessage *msg) override;
    //@}

    virtual void requestStream();
    virtual void receiveStream(Packet *msg);
    virtual void timeOutData();

    // ApplicationBase:
    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

    virtual void socketDataArrived(UdpSocket *socket, Packet *packet) override;
    virtual void socketErrorArrived(UdpSocket *socket, Indication *indication) override;
    virtual void socketClosed(UdpSocket *socket) override;

  public:
    UdpVideoStreamClient2();
    virtual ~UdpVideoStreamClient2();

};

}

#endif

