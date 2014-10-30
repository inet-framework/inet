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

#ifndef __INET_CONSTANTGAINANTENNA_H
#define __INET_CONSTANTGAINANTENNA_H

#include "inet/physicallayer/base/AntennaBase.h"

namespace inet {

namespace physicallayer {

class INET_API ConstantGainAntenna : public AntennaBase
{
  protected:
    double gain;

  protected:
    virtual void initialize(int stage);

  public:
    ConstantGainAntenna();

    virtual void printToStream(std::ostream& stream) const;
    virtual double getMaxGain() const { return gain; }
    virtual double computeGain(const EulerAngles direction) const { return gain; }
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_CONSTANTGAINANTENNA_H

