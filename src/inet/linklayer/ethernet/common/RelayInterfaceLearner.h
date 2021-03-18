//
// Copyright (C) 2020 OpenSim Ltd.
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
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#ifndef __INET_RELAYINTERFACELEARNER_H
#define __INET_RELAYINTERFACELEARNER_H

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ModuleRefByPar.h"
#include "inet/linklayer/ethernet/contract/IMacAddressTable.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/queueing/base/PacketFlowBase.h"

namespace inet {

using namespace inet::queueing;

class INET_API RelayInterfaceLearner : public PacketFlowBase, public TransparentProtocolRegistrationListener
{
  protected:
    ModuleRefByPar<IInterfaceTable> interfaceTable;
    ModuleRefByPar<IMacAddressTable> macAddressTable;

  protected:
    virtual void initialize(int stage) override;

  protected:
    virtual void processPacket(Packet *packet) override;

    virtual bool isForwardingService(const Protocol& protocol, cGate *gate, ServicePrimitive servicePrimitive) const override { return false; }
    virtual bool isForwardingServiceGroup(const ProtocolGroup& protocolGroup, cGate *gate, ServicePrimitive servicePrimitive) const override { return false; }
    virtual bool isForwardingAnyService(cGate *gate, ServicePrimitive servicePrimitive) const override { return false; }

    virtual bool isForwardingProtocol(const Protocol& protocol, cGate *gate, ServicePrimitive servicePrimitive) const override { return servicePrimitive == SP_INDICATION; }
    virtual bool isForwardingProtocolGroup(const ProtocolGroup& protocolGroup, cGate *gate, ServicePrimitive servicePrimitive) const override { return servicePrimitive == SP_INDICATION; }
    virtual bool isForwardingAnyProtocol(cGate *gate, ServicePrimitive servicePrimitive) const override { return servicePrimitive == SP_INDICATION; }

    virtual cGate *getRegistrationForwardingGate(cGate *gate) override;
};

} // namespace inet

#endif

