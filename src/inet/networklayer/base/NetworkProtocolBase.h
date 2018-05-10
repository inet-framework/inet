//
// Copyright (C) 2013 OpenSim Ltd
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

#ifndef __INET_NETWORKPROTOCOLBASE_H
#define __INET_NETWORKPROTOCOLBASE_H

#include "inet/common/LayeredProtocolBase.h"
#include "inet/common/lifecycle/NodeOperations.h"
#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include <map>
#include <set>

namespace inet {

class INET_API NetworkProtocolBase : public LayeredProtocolBase, public IProtocolRegistrationListener
{
  protected:
    struct SocketDescriptor
    {
        int socketId = -1;
        int protocolId = -1;
        L3Address localAddress;
        L3Address remoteAddress;

        SocketDescriptor(int socketId, int protocolId, L3Address localAddress)
                : socketId(socketId), protocolId(protocolId), localAddress(localAddress) { }
    };

    IInterfaceTable *interfaceTable;
    // working vars
    std::set<const Protocol *> upperProtocols;
    std::map<int, SocketDescriptor *> socketIdToSocketDescriptor;

  protected:
    NetworkProtocolBase();
    virtual ~NetworkProtocolBase() { for (auto entry : socketIdToSocketDescriptor) delete entry.second; }

    virtual void initialize(int stage) override;

    virtual void sendUp(cMessage *message);
    virtual void sendDown(cMessage *message, int interfaceId = -1);

    virtual bool isUpperMessage(cMessage *message) override;
    virtual bool isLowerMessage(cMessage *message) override;

    virtual bool isInitializeStage(int stage) override { return stage == INITSTAGE_NETWORK_LAYER; }
    virtual bool isNodeStartStage(int stage) override { return stage == NodeStartOperation::STAGE_NETWORK_LAYER; }
    virtual bool isNodeShutdownStage(int stage) override { return stage == NodeShutdownOperation::STAGE_NETWORK_LAYER; }

    virtual void handleUpperCommand(cMessage *msg) override;

    virtual const Protocol& getProtocol() const = 0;

  public:
    virtual void handleRegisterService(const Protocol& protocol, cGate *out, ServicePrimitive servicePrimitive) override;
    virtual void handleRegisterProtocol(const Protocol& protocol, cGate *in, ServicePrimitive servicePrimitive) override;
};

} // namespace inet

#endif // ifndef __INET_NETWORKPROTOCOLBASE_H

