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
#include "inet/common/ProtocolMap.h"
#include "inet/networklayer/common/IInterfaceTable.h"

namespace inet {

class INET_API NetworkProtocolBase : public LayeredProtocolBase
{
  protected:
    ProtocolMapping protocolMapping;
    IInterfaceTable *interfaceTable;

  protected:
    NetworkProtocolBase();

    virtual void initialize(int stage);

    virtual void handleUpperCommand(cMessage *message);

    virtual void sendUp(cMessage *message, int transportProtocol);
    virtual void sendDown(cMessage *message, int interfaceId = -1);

    virtual bool isUpperMessage(cMessage *message);
    virtual bool isLowerMessage(cMessage *message);

    virtual bool isInitializeStage(int stage) { return stage == INITSTAGE_NETWORK_LAYER; }
    virtual bool isNodeStartStage(int stage) { return stage == NodeStartOperation::STAGE_NETWORK_LAYER; }
    virtual bool isNodeShutdownStage(int stage) { return stage == NodeShutdownOperation::STAGE_NETWORK_LAYER; }
};

} // namespace inet

#endif // ifndef __INET_NETWORKPROTOCOLBASE_H

