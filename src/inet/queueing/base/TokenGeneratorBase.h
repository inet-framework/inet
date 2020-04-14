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

#ifndef __INET_TOKENGENERATORBASE_H
#define __INET_TOKENGENERATORBASE_H

#include "inet/common/StringFormat.h"
#include "inet/queueing/contract/IPacketProcessor.h"
#include "inet/queueing/server/TokenBasedServer.h"

namespace inet {
namespace queueing {

class INET_API TokenGeneratorBase : public PacketQueueingElementBase, public StringFormat::IDirectiveResolver
{
  public:
    static simsignal_t tokensCreatedSignal;

  protected:
    const char *displayStringTextFormat = nullptr;
    TokenBasedServer *server = nullptr;
    int numTokensGenerated = -1;

  protected:
    virtual void initialize(int stage) override;

    virtual void updateDisplayString();

  public:
    virtual bool supportsPacketPushing(cGate *gate) const override { return false; }
    virtual bool supportsPacketPulling(cGate *gate) const override { return false; }

    virtual const char *resolveDirective(char directive) const override;
};

} // namespace queueing
} // namespace inet

#endif // ifndef __INET_TOKENGENERATORBASE_H

