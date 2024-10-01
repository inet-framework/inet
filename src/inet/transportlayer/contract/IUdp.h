//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IUDP_H
#define __INET_IUDP_H

#include "inet/networklayer/common/L3Address.h"

namespace inet {

class INET_API IUdp
{
  public:
    class ICallback {
      public:
        virtual void handleClose() = 0;
        virtual void handleError(Indication *indication) = 0;
    };

  public:
    virtual void setCallback(int socketId, ICallback *callback) = 0;
    virtual void bind(int socketId, const L3Address& localAddr, int localPort) = 0;
    virtual void connect(int socketId, const L3Address& remoteAddr, int remotePort) = 0;
    virtual void setBroadcast(int socketId, bool broadcast) = 0;
    virtual void setMulticastLoop(int socketId, bool value) = 0;
    virtual void setTimeToLive(int socketId, int ttl) = 0;
    virtual void setDscp(int socketId, short dscp) = 0;
    virtual void setTos(int socketId, short dscp) = 0;
    virtual void joinMulticastGroups(int socketId, const std::vector<L3Address>& multicastAddresses, const std::vector<int> interfaceIds) = 0;
    virtual void leaveMulticastGroups(int socketId, const std::vector<L3Address>& multicastAddresses) = 0;
    virtual void close(int socketId) = 0;
    virtual void destroy(int socketId) = 0;
};

} // namespace inet

#endif

