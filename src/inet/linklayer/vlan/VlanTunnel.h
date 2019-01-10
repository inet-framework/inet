//
// Copyright (C) OpenSim Ltd.
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

#ifndef __INET_VLANTUNNEL_H
#define __INET_VLANTUNNEL_H

#include "inet/linklayer/ethernet/EthernetSocket.h"
#include "inet/networklayer/common/InterfaceEntry.h"

namespace inet {

class VlanTunnel : public cSimpleModule, public EthernetSocket::ICallback
{
  protected:
    int vlanId = -1;
    InterfaceEntry *realInterfaceEntry = nullptr;
    InterfaceEntry *interfaceEntry = nullptr;
    EthernetSocket socket;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;

    virtual void socketDataArrived(EthernetSocket *socket, Packet *packet) override;
    virtual void socketErrorArrived(EthernetSocket *socket, Indication *indication) override;
    virtual void socketClosed(EthernetSocket *socket) override {}
};

} // namespace inet

#endif // ifndef __INET_VLANTUNNEL_H

