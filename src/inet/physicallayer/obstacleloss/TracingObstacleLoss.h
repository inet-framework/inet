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

#ifndef __INET_TRACINGOBSTACLELOSS_H
#define __INET_TRACINGOBSTACLELOSS_H

#include "inet/common/IVisitor.h"
#include "inet/common/figures/TrailFigure.h"
#include "inet/environment/contract/IPhysicalEnvironment.h"
#include "inet/physicallayer/contract/packetlevel/IRadioMedium.h"
#include "inet/physicallayer/contract/packetlevel/IObstacleLoss.h"

namespace inet {

namespace physicallayer {

/**
 * This class computes obstacle loss based on the actual straight path that the
 * radio signal travels from the transmitter to the receiver. The total loss is
 * the combination of the dielectric losses in the intersected obstacles and the
 * reflection losses of the penetrated faces along this path.
 */
class INET_API TracingObstacleLoss : public cModule, public IObstacleLoss
{
  protected:
    class TotalObstacleLossComputation : public IVisitor
    {
      protected:
        mutable double totalLoss;
        const TracingObstacleLoss *obstacleLoss;
        const Hz frequency;
        const Coord transmissionPosition;
        const Coord receptionPosition;

      public:
        TotalObstacleLossComputation(const TracingObstacleLoss *obstacleLoss, Hz frequency, const Coord& transmissionPosition, const Coord& receptionPosition);
        void visit(const cObject *object) const override;
        double getTotalLoss() const { return totalLoss; }
    };

  protected:
    /** @name Parameters */
    //@{
    /**
     * The radio medium where the radio signal propagation takes place.
     */
    IRadioMedium *medium;
    /**
     * The physical environment that provides to obstacles.
     */
    IPhysicalEnvironment *environment;
    /**
     * Leaves graphical trail of obstacle intersections.
     */
    bool leaveIntersectionTrail;
    /**
     * Leaves graphical trail of intersection face normal vectors.
     */
    bool leaveFaceNormalVectorTrail;
    //@}

    /** @name Graphics */
    //@{
    /**
     * The trail figures that represent the last couple of obstacle intersections.
     */
    TrailFigure *intersectionTrail;
    //@}

    /** @name Statistics */
    //@{
    /**
     * Total number of obstacle intersection computations.
     */
    mutable unsigned int intersectionComputationCount;
    /**
     * Total number of actual obstacle intersections.
     */
    mutable unsigned int intersectionCount;
    //@}

  protected:
    virtual void initialize(int stage) override;
    virtual void finish() override;

    virtual double computeDielectricLoss(const IMaterial *material, Hz frequency, m distance) const;
    virtual double computeReflectionLoss(const IMaterial *incidentMaterial, const IMaterial *refractiveMaterial, double angle) const;
    virtual double computeObjectLoss(const IPhysicalObject *object, Hz frequency, const Coord& transmissionPosition, const Coord& receptionPosition) const;

  public:
    TracingObstacleLoss();
    virtual std::ostream& printToStream(std::ostream& stream, int level) const override;
    virtual double computeObstacleLoss(Hz frequency, const Coord& transmissionPosition, const Coord& receptionPosition) const override;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_TRACINGOBSTACLELOSS_H

