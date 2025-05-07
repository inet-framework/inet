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

#ifndef __INET_QUIC_QUIC_H_
#define __INET_QUIC_QUIC_H_

#include <omnetpp.h>
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/lifecycle/OperationalBase.h"
#include "Connection.h"
#include "UdpSocket.h"
#include "AppSocket.h"
#include "inet/networklayer/contract/IRoutingTable.h"

using namespace omnetpp;

namespace inet {
namespace quic {

// Forward declarations:
class Connection;
class UdpSocket;

class Quic : public OperationalBase
{
  public:
    virtual ~Quic();
    virtual Connection *createConnection(UdpSocket *udpSocket, AppSocket *appSocket, L3Address remoteAddr, uint16_t remotePort);
    virtual UdpSocket *createUdpSocket();
    virtual UdpSocket *findUdpSocket(L3Address addr, uint16_t port);
    virtual AppSocket *findOrCreateAppSocket(int socketId);
    IRoutingTable *getRoutingTable();

  protected:
    virtual void handleMessageWhenUp(cMessage *msg) override;

    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;
    virtual bool isInitializeStage(int stage) const override { return stage == INITSTAGE_TRANSPORT_LAYER; }
    virtual bool isModuleStartStage(int stage) const override { return stage == ModuleStartOperation::STAGE_TRANSPORT_LAYER; }
    virtual bool isModuleStopStage(int stage) const override { return stage == ModuleStopOperation::STAGE_TRANSPORT_LAYER; }

  private:
    std::map<uint64_t, Connection *> connectionIdConnectionMap;
    std::map<int, UdpSocket *> udpSocketIdUdpSocketMap;
    std::map<int, AppSocket *> socketIdAppSocketMap;

    void handleTimeout(cMessage *msg);
    void handleMessageFromApp(cMessage *msg);
    void handleMessageFromUdp(cMessage *msg);
    void addConnection(Connection *connection);
    void addUdpSocket(UdpSocket *udpSocket);
    Connection *findConnection(uint64_t srcConnectionId);
    Connection *findConnectionByDstConnectionId(uint64_t connectionId, Packet *pkt);
    UdpSocket *findUdpSocket(int socketId);
    uint64_t extractConnectionId(Packet *pkt, bool *readSourceConnectionId = nullptr);
};

} //namespace quic
} //namespace inet

#endif
