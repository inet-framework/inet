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
#include "TrailFigure.h"
#include "SpatialGrid.h"
#include "RadioMedium.h"

namespace inet {

namespace physicallayer {

// TODO: allow dB attenuation per meter/per wall
class INET_API ObstacleLoss : public cModule, public IObstacleLoss
{

  protected:
    class ObstacleLossVisitor : public IVisitor
    {
        protected:
            mutable double totalLoss;
            const ObstacleLoss *obstacleLoss;
            Hz frequency;
            Coord transmissionPosition;
            Coord receptionPosition;
        public:
            ObstacleLossVisitor(const ObstacleLoss *obstacleLoss, Hz frequency, const Coord& transmissionPosition, const Coord& receptionPosition) :
                totalLoss(1), obstacleLoss(obstacleLoss), frequency(frequency),
                transmissionPosition(transmissionPosition),
                receptionPosition(receptionPosition) {}
            void visit(const cObject *object) const;
            double getTotalLoss() const { return totalLoss; }
    };

  protected:
    IRadioMedium *medium;
    PhysicalEnvironment *environment;
    bool leaveIntersectionTrail;
    TrailFigure *intersectionTrail;
    mutable unsigned int intersectionComputationCount;
    mutable unsigned int intersectionCount;

  protected:
    virtual void initialize(int stage);
    virtual void finish();

    virtual double computeDielectricLoss(const Material *material, Hz frequency, m distance) const;
    virtual double computeReflectionLoss(const Material *incidentMaterial, const Material *refractiveMaterial, double angle) const;
    // TODO: revise name
    void obstacleLoss(const PhysicalObject *object, Hz frequency, const Coord& transmissionPosition, const Coord& receptionPosition, double& totalLoss) const;
  public:
    ObstacleLoss();
    virtual ~ObstacleLoss();
    virtual void printToStream(std::ostream& stream) const;
    virtual double computeObstacleLoss(Hz frequency, const Coord& transmissionPosition, const Coord& receptionPosition) const;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_OBSTACLELOSS_H

