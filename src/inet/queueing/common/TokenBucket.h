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

#ifndef __INET_TOKENBUCKET_H
#define __INET_TOKENBUCKET_H

#include "inet/common/packet/Packet.h"
#include "inet/queueing/contract/ITokenStorage.h"

namespace inet {
namespace queueing {

class INET_API TokenBucket : public ITokenStorage
{
  protected:
    double numTokens = 0;
    double maxNumTokens = -1;
    double tokenProductionRate = 0;
    double excessTokenProductionRate = 0;
    ITokenStorage *excessTokenStorage = nullptr;

    simtime_t lastUpdate;

  protected:
    void updateNumTokens();

  public:
    TokenBucket() {}
    TokenBucket(double numTokens, double maxNumTokens, double tokenProductionRate, ITokenStorage *excessTokenStorage);

    virtual ITokenStorage *getExcessTokenStorage() const { return excessTokenStorage; }
    virtual double getNumTokens() const override;
    virtual void addTokens(double numTokens) override;
    virtual void removeTokens(double numTokens) override;
    virtual void addTokenProductionRate(double tokenRate) override;
    virtual void removeTokenProductionRate(double tokenRate) override;

    /**
     * Returns the simulation time when the token bucket becomes full with the
     * current rate of token production.
     */
    virtual simtime_t getOverflowTime();
};

inline std::ostream& operator<<(std::ostream& out, const TokenBucket& tokenBucket)
{
    return out << tokenBucket.getNumTokens();
}

} // namespace queueing
} // namespace inet

#endif

