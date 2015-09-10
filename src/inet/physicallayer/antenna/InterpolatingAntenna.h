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

#include "inet/physicallayer/base/packetlevel/AntennaBase.h"

namespace inet {

namespace physicallayer {

class INET_API InterpolatingAntenna : public AntennaBase
{
  protected:
    double minGain;
    double maxGain;
    std::map<double, double> elevationGainMap;
    std::map<double, double> headingGainMap;
    std::map<double, double> bankGainMap;

  protected:
    virtual void initialize(int stage) override;
    virtual void parseMap(std::map<double, double>& gainMap, const char *text);
    virtual double computeGain(const std::map<double, double>& gainMap, double angle) const;

  public:
    InterpolatingAntenna();

    virtual std::ostream& printToStream(std::ostream& stream, int level) const override;
    virtual double getMinGain() const { return minGain; }
    virtual double getMaxGain() const override { return maxGain; }
    virtual double computeGain(const EulerAngles direction) const override;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_INTERPOLATINGANTENNA_H

