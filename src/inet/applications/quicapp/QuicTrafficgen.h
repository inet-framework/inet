//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
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
      virtual void socketAvailable(QuicSocket *socket, QuicAvailableInfo *availableInfo) override { };
      virtual void socketEstablished(QuicSocket *socket) override;
      virtual void socketClosed(QuicSocket *socket) override;
      virtual void socketDeleted(QuicSocket *socket) override { };

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
