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

#ifndef __INET_OBSTACLELOSS_H
#define __INET_OBSTACLELOSS_H

#include "IObstacleLoss.h"
#include "PhysicalEnvironment.h"

namespace inet {

namespace physicallayer {
// TODO: add reflection from walls
// TODO: allow dB attenuation per meter/per wall
// TODO: fix problem when one end is in the obstacle
class INET_API ObstacleLoss : public cModule, public IObstacleLoss
{
  protected:
    PhysicalEnvironment *environment;

  protected:
    virtual void initialize(int stage);
    virtual double computeDielectricLoss(const PhysicalObject *object, Hz frequency, const Coord transmissionPosition, const Coord receptionPosition) const;

  public:
    ObstacleLoss();

    virtual void printToStream(std::ostream& stream) const;
    virtual double computeObstacleLoss(Hz frequency, const Coord transmissionPosition, const Coord receptionPosition) const;
};

} // namespace physicallayer
} // namespace inet

#endif // ifndef __INET_OBSTACLELOSS_H

