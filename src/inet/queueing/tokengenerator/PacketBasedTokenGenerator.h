//
// Copyright (C) OpenSim Ltd.
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
// along with this program; if not, see http://www.gnu.org/licenses/.
//

#ifndef __INET_PACKETBASEDTOKENGENERATOR_H
#define __INET_PACKETBASEDTOKENGENERATOR_H

#include "inet/queueing/base/PassivePacketSinkBase.h"
#include "inet/queueing/server/TokenBasedServer.h"

namespace inet {
namespace queueing {

class INET_API PacketBasedTokenGenerator : public PassivePacketSinkBase, public cListener
{
  protected:
    cPar *numTokensPerPacketParameter = nullptr;
    cPar *numTokensPerBitParameter = nullptr;

    cGate *inputGate = nullptr;
    IActivePacketSource *producer = nullptr;
    TokenBasedServer *server = nullptr;

    int numTokensGenerated = -1;

  protected:
    virtual void initialize(int stage) override;

  public:
    virtual bool supportsPacketPushing(cGate *gate) const override { return true; }
    virtual bool supportsPacketPulling(cGate *gate) const override { return false; }

    virtual bool canPushSomePacket(cGate *gate) const override { return server->getNumTokens() == 0; }
    virtual bool canPushPacket(Packet *packet, cGate *gate) const override { return server->getNumTokens() == 0; }
    virtual void pushPacket(Packet *packet, cGate *gate) override;

    virtual const char *resolveDirective(char directive) const override;
    virtual void receiveSignal(cComponent *source, simsignal_t signal, double value, cObject *details) override;
};

} // namespace queueing
} // namespace inet

#endif // ifndef __INET_PACKETBASEDTOKENGENERATOR_H

