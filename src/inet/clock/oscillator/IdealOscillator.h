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

#ifndef __INET_IDEALOSCILLATOR_H
#define __INET_IDEALOSCILLATOR_H

#include "inet/clock/base/OscillatorBase.h"
#include "inet/common/INETMath.h"

namespace inet {

class INET_API IdealOscillator : public OscillatorBase
{
  protected:
    simtime_t origin;
    simtime_t tickLength;

  protected:
    virtual void initialize(int stage) override;
    virtual void finish() override;

  public:
    virtual simtime_t getComputationOrigin() const override { return origin; }
    virtual simtime_t getNominalTickLength() const override { return tickLength; }

    virtual int64_t computeTicksForInterval(simtime_t timeInterval) const override;
    virtual simtime_t computeIntervalForTicks(int64_t numTicks) const override;
};

} // namespace inet

#endif

