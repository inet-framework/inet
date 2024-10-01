//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ISCTP_H
#define __INET_ISCTP_H

#include "inet/common/packet/Message.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/transportlayer/contract/sctp/SctpCommand_m.h"

namespace inet {
namespace sctp {

class INET_API ISctp
{
  public:
    class ICallback {
      public:
        virtual ~ICallback() {}
        virtual void handleEstablished(Indication *indication) = 0;
        virtual void handleAvailable(Indication *indication) = 0;
        virtual void handleDataArrived(Packet *packet, bool urgent) = 0;
        virtual void handleDataArrivedNotification(Indication *indication) = 0;
        virtual void handleClosed() = 0;
        virtual void handlePeerClosed() = 0;
        virtual void handleFailure(int code) = 0;
        virtual void handleShutdownReceived() = 0;
        virtual void handleAbandoned() = 0;
        virtual void handleSendStreamReset() = 0;
        virtual void handleReceiveStreamReset() = 0;
        virtual void handleSendMessage() = 0;
        virtual void handleStatus(Indication *indication) = 0;
        virtual void handleAddressAdded(L3Address localAddr, L3Address remoteAddr) = 0;
    };

  public:
    virtual void setCallback(int socketId, ICallback *callback) = 0;
    virtual void listen(int socketId, const std::vector<L3Address>& localAddresses, int localPort, bool fork, int inboundStreams, int outboundStreams, bool streamReset, uint32_t requests, uint32_t messagesToPush) = 0;
    virtual void connect(int socketId, const std::vector<L3Address>& localAddresses, int localPort, L3Address remoteAddress, int32_t remotePort, int inboundStreams, int outboundStreams, bool streamReset, int32_t prMethod, uint32_t numRequests) = 0;
    virtual void accept(int socketId) = 0;
    virtual void abort(int socketId) = 0;
    virtual void close(int socketId, int id) = 0;
    virtual void shutdown(int socketId, int id) = 0;
    virtual void receive(int socketId, int sid, int numMsgs) = 0;
    virtual void streamReset(int socketId, L3Address remoteAddress, int type, int stream) = 0;
    virtual void getSocketOptions(int socketId) = 0;
    virtual void setQueueLimits(int socketId, int packetCapacity, B dataCapacity) = 0;
};

} // namespace sctp
} // namespace inet

#endif

