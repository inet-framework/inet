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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

#ifndef __INET_TIMEBASEDTOKENGENERATOR_H
#define __INET_TIMEBASEDTOKENGENERATOR_H

#include "inet/queueing/base/TokenGeneratorBase.h"

namespace inet {
namespace queueing {

class INET_API TimeBasedTokenGenerator : public TokenGeneratorBase
{
  protected:
    cPar *generationIntervalParameter = nullptr;
    cPar *numTokensParameter = nullptr;

    cMessage *generationTimer = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;

    virtual void scheduleGenerationTimer();

  public:
    virtual ~TimeBasedTokenGenerator() { cancelAndDelete(generationTimer); }

    virtual bool supportsPacketPushing(cGate *gate) const override { return false; }
    virtual bool supportsPacketPulling(cGate *gate) const override { return false; }
};

} // namespace queueing
} // namespace inet

#endif

