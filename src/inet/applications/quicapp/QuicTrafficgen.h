//
// Copyright (C) 2019-2024 Timo Völker
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_QUIC_QUICTRAFFICGEN_H_
#define __INET_QUIC_QUICTRAFFICGEN_H_

#include <omnetpp.h>
#include "inet/transportlayer/contract/quic/QuicSocket.h"
#include "inet/applications/base/ApplicationBase.h"
#include "inet/applications/trafficgen/TrafficgenMessage_m.h"

using namespace omnetpp;

namespace inet {

class QuicTrafficgen : public ApplicationBase, public QuicSocket::ICallback
{
    public:
      QuicTrafficgen();
      ~QuicTrafficgen();

    protected:
      QuicSocket socket;

      virtual void handleMessageWhenUp(cMessage *msg) override;

      virtual void handleStartOperation(LifecycleOperation *operation) override;
      virtual void handleStopOperation(LifecycleOperation *operation) override;
      virtual void handleCrashOperation(LifecycleOperation *operation) override;

      virtual void socketDataArrived(QuicSocket* socket, Packet *packet) override;
      virtual void socketConnectionAvailable(QuicSocket *socket) override { };
      virtual void socketDataAvailable(QuicSocket* socket, QuicDataInfo *dataInfo) override { };
      virtual void socketEstablished(QuicSocket *socket) override;
      virtual void socketClosed(QuicSocket *socket) override;
      virtual void socketDestroyed(QuicSocket *socket) override { };

      virtual void socketSendQueueFull(QuicSocket *socket) override;
      virtual void socketSendQueueDrain(QuicSocket *socket) override;
      virtual void socketMsgRejected(QuicSocket *socket) override { };
      virtual void sendData(TrafficgenData* gmsg);
      virtual uint32_t getStreamId(int generatorId);

    private:
      struct Stream {
          uint32_t streamId;
          uint32_t priority;
          bool finished;
          uint64_t msgCount;
      };

      enum Timer {
          TIMER_CONNECT,
          TIMER_RESET,
          TIMER_LIMIT_RUNTIME
      };

      cMessage *timerConnect;
      cMessage *timerLimitRuntime;
      unsigned int generatorsActive;
      std::map<uint32_t,Stream> streams;
      L3Address connectAddress;
      unsigned int connectPort;
      bool sendingAllowed = false;

      void handleTimeout(cMessage *msg);
      void handleMessageFromGenerator(cMessage *msg);
      void handleGeneratorInfo(TrafficgenInfo* msg);
      void sendGeneratorControl(uint8_t controlMessageType);
      void setStatusString(const char *s);
      void close();
};

} //namespace

#endif
