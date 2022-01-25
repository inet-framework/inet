//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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

#ifndef __INET_TOKENBASEDSERVER_H
#define __INET_TOKENBASEDSERVER_H

#include "inet/queueing/base/PacketServerBase.h"
#include "inet/queueing/contract/ITokenStorage.h"

namespace inet {
namespace queueing {

class INET_API TokenBasedServer : public PacketServerBase, public ITokenStorage
{
  protected:
    cPar *tokenConsumptionPerPacketParameter = nullptr;
    cPar *tokenConsumptionPerBitParameter = nullptr;
    double maxNumTokens = NaN;

    bool tokensDepletedSignaled = true;
    double numTokens = 0;

  protected:
    virtual void initialize(int stage) override;
    virtual void processPackets();

  public:
    virtual double getNumTokens() const override { return numTokens; }
    virtual void addTokens(double tokens) override;
    virtual void removeTokens(double tokens) override { throw cRuntimeError("TODO"); }
    virtual void addTokenProductionRate(double tokenRate) override { throw cRuntimeError("TODO"); }
    virtual void removeTokenProductionRate(double tokenRate) override { throw cRuntimeError("TODO"); }

    virtual void handleCanPushPacketChanged(cGate *gate) override;
    virtual void handleCanPullPacketChanged(cGate *gate) override;

    virtual const char *resolveDirective(char directive) const override;
};

} // namespace queueing
} // namespace inet

#endif

