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

#ifndef __INET_INTERPOLATINGANTENNA_H
#define __INET_INTERPOLATINGANTENNA_H

#include "inet/common/INETDefs.h"
#include "inet/physicallayer/base/packetlevel/AntennaBase.h"

namespace inet {

namespace physicallayer {

/**
 * TODO: refactor to use Delaunay triangulation on unit sphere, i.e. triangulate
 * result from enclosing spherical triangle as seen from the center
 */
class INET_API InterpolatingAntenna : public AntennaBase
{
  protected:
    class AntennaGain : public IAntennaGain
    {
      protected:
        double minGain;
        double maxGain;
        std::map<rad, double> elevationGainMap;
        std::map<rad, double> headingGainMap;
        std::map<rad, double> bankGainMap;

      protected:
        virtual void parseMap(std::map<rad, double>& gainMap, const char *text);
        virtual double computeGain(const std::map<rad, double>& gainMap, rad angle) const;

      public:
        AntennaGain(const char *elevation, const char *heading, const char *bank);
        virtual double getMinGain() const override { return minGain; }
        virtual double getMaxGain() const override { return maxGain; }
        virtual double computeGain(const Quaternion direction) const override;
    };

    Ptr<AntennaGain> gain;

  protected:
    virtual void initialize(int stage) override;

  public:
    InterpolatingAntenna();

    virtual std::ostream& printToStream(std::ostream& stream, int level) const override;
    virtual Ptr<const IAntennaGain> getGain() const override { return gain; }
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_INTERPOLATINGANTENNA_H

