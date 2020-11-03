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

#ifndef __INET_CONSTANTDRIFTOSCILLATOR_H
#define __INET_CONSTANTDRIFTOSCILLATOR_H

#include "inet/clock/base/OscillatorBase.h"
#include "inet/common/INETMath.h"
#include "inet/common/scenario/IScriptable.h"

namespace inet {

class INET_API ConstantDriftOscillator : public OscillatorBase, public IScriptable
{
  protected:
    simtime_t nominalTickLength;
    double relativeSpeed = NaN;
    double driftRate = NaN;

    simtime_t origin; // simulation time from which the computeClockTicksForInterval and computeIntervalForClockTicks is measured
    simtime_t nextTickFromOrigin; // simulation time interval from the computation origin to the next tick

  protected:
    virtual void initialize(int stage) override;

    // IScriptable implementation
    virtual void processCommand(const cXMLElement& node) override;

  public:
    virtual simtime_t getComputationOrigin() const override { return origin; }
    virtual simtime_t getNominalTickLength() const override { return nominalTickLength; }
    virtual simtime_t getCurrentTickLength() const { return nominalTickLength / (1.0 + driftRate); }

    virtual void setDriftRate(double driftRate);
    virtual void setTickOffset(simtime_t tickOffset);

    virtual int64_t computeTicksForInterval(simtime_t timeInterval) const override;
    virtual simtime_t computeIntervalForTicks(int64_t numTicks) const override;

    virtual const char *resolveDirective(char directive) const override;
};

} // namespace inet

#endif

