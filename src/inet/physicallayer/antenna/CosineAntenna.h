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

#ifndef __INET_COSINEANTENNA_H
#define __INET_COSINEANTENNA_H

#include "inet/common/INETDefs.h"
#include "inet/physicallayer/base/packetlevel/AntennaBase.h"

namespace inet {

namespace physicallayer {

class INET_API CosineAntenna : public AntennaBase
{
  protected:
    virtual void initialize(int stage) override;

    class AntennaGain : public IAntennaGain
    {
      public:
        AntennaGain(double maxGain, deg beamWidth);
        virtual double getMinGain() const override { return 0; }
        virtual double getMaxGain() const override { return maxGain; }
        virtual deg getBeamWidth() const { return beamWidth; }
        virtual double computeGain(const Quaternion direction) const override;

      protected:
        double maxGain;
        deg beamWidth;
    };

    Ptr<AntennaGain> gain;

  public:
    CosineAntenna();

    virtual std::ostream& printToStream(std::ostream& stream, int level) const override;
    virtual Ptr<const IAntennaGain> getGain() const override { return gain; }
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_COSINEANTENNA_H

