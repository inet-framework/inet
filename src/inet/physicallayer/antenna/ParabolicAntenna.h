//
// Copyright (C) 2013 OpenSim Ltd.
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
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_PARABOLICANTENNA_H
#define __INET_PARABOLICANTENNA_H

#include "inet/common/INETDefs.h"
#include "inet/physicallayer/base/packetlevel/AntennaBase.h"

namespace inet {

namespace physicallayer {

class INET_API ParabolicAntenna : public AntennaBase
{
  protected:
    class AntennaGain : public IAntennaGain
    {
      public:
        AntennaGain(double maxGain, double minGain, deg beamWidth);
        virtual double getMaxGain() const override { return maxGain; }
        virtual double getMinGain() const override { return minGain; }
        virtual deg getBeamWidth() const { return beamWidth; }
        virtual double computeGain(const Quaternion direction) const override;

      protected:
        double maxGain;
        double minGain;
        deg beamWidth;
    };

    Ptr<AntennaGain> gain;

  protected:
    virtual void initialize(int stage) override;

  public:
    ParabolicAntenna();

    virtual std::ostream& printToStream(std::ostream& stream, int level) const override;
    virtual Ptr<const IAntennaGain> getGain() const override { return gain; }
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_PARABOLICANTENNA_H

