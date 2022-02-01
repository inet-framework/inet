//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

// Based on the video streaming app of the similar name by Johnny Lai.
//

#ifndef __INET_UDPVIDEOSTREAMCLIENT_H
#define __INET_UDPVIDEOSTREAMCLIENT_H

#include "inet/applications/base/ApplicationBase.h"
#include "inet/common/packet/Packet.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"

namespace inet {

/**
 * A "Realtime" VideoStream client application.
 *
 * Basic video stream application. Clients connect to server and get a stream of
 * video back.
 */
class INET_API UdpVideoStreamClient : public ApplicationBase, public UdpSocket::ICallback
{
  protected:

    // state
    UdpSocket socket;
    cMessage *selfMsg = nullptr;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void finish() override;
    virtual void handleMessageWhenUp(cMessage *msg) override;

    virtual void requestStream();
    virtual void receiveStream(Packet *msg);

    // ApplicationBase:
    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

    virtual void socketDataArrived(UdpSocket *socket, Packet *packet) override;
    virtual void socketErrorArrived(UdpSocket *socket, Indication *indication) override;
    virtual void socketClosed(UdpSocket *socket) override;

  public:
    UdpVideoStreamClient() {}
    virtual ~UdpVideoStreamClient() { cancelAndDelete(selfMsg); }
};

} // namespace inet

#endif

