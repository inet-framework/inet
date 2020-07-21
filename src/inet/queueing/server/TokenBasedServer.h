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

#ifndef __INET_TOKENBASEDSERVER_H
#define __INET_TOKENBASEDSERVER_H

#include "inet/queueing/base/PacketServerBase.h"

namespace inet {
namespace queueing {

class INET_API TokenBasedServer : public PacketServerBase
{
  public:
    static simsignal_t tokensAddedSignal;
    static simsignal_t tokensRemovedSignal;
    static simsignal_t tokensDepletedSignal;

  protected:
    cPar *tokenConsumptionPerPacketParameter = nullptr;
    cPar *tokenConsumptionPerBitParameter = nullptr;
    const char *displayStringTextFormat = nullptr;
    double maxNumTokens = NaN;

    bool tokensDepletedSignaled = true;
    double numTokens = 0;

  protected:
    virtual void initialize(int stage) override;
    virtual void processPackets();

  public:
    virtual int getNumTokens() const { return numTokens; }
    virtual void addTokens(double tokens);

    virtual void handleCanPushPacketChanged(cGate *gate) override;
    virtual void handleCanPullPacket(cGate *gate) override;

    virtual const char *resolveDirective(char directive) const override;
};

} // namespace queueing
} // namespace inet

#endif // ifndef __INET_TOKENBASEDSERVER_H

