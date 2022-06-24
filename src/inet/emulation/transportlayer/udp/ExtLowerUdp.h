//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_EXTLOWERUDP_H
#define __INET_EXTLOWERUDP_H

#include "inet/common/lifecycle/ILifecycle.h"
#include "inet/common/packet/printer/PacketPrinter.h"
#include "inet/common/scheduler/RealTimeScheduler.h"
#include "inet/networklayer/common/L3Address.h"

namespace inet {

class INET_API ExtLowerUdp : public cSimpleModule, public ILifecycle, public RealTimeScheduler::ICallback
{
  protected:
    class INET_API Socket {
      public:
        int socketId = -1;
        int fd = -1;

      public:
        Socket(int socketId) : socketId(socketId) {}
    };

    const char *packetNameFormat = nullptr;
    PacketPrinter packetPrinter;
    RealTimeScheduler *rtScheduler = nullptr;
    std::map<int, Socket *> socketIdToSocketMap;
    std::map<int, Socket *> fdToSocketMap;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;
    virtual bool handleOperationStage(LifecycleOperation *operation, IDoneCallback *doneCallback) override;
    virtual bool notify(int fd) override;

    virtual Socket *open(int socketId);
    virtual void bind(int socketId, const L3Address& localAddress, int localPort);
    virtual void connect(int socketId, const L3Address& remoteAddress, int remotePort);
    virtual void close(int socketId);
    virtual void processPacketFromUpper(Packet *packet);
    virtual void processPacketFromLower(int fd);

  public:
    virtual ~ExtLowerUdp();
};

} // namespace inet

#endif

