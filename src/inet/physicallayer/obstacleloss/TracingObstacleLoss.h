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

#include <algorithm>
#include "inet/common/IVisitor.h"
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
  public:
    class INET_API ITracingObstacleLossListener {
      public:
        virtual void obstaclePenetrated(const IPhysicalObject *object, const Coord& intersection1, const Coord& intersection2, const Coord& normal1, const Coord& normal2) = 0;
    };

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
    //@}

    /** @name Statistics */
    //@{
    std::vector<ITracingObstacleLossListener *> listeners;
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
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void finish() override;

    virtual double computeDielectricLoss(const IMaterial *material, Hz frequency, m distance) const;
    virtual double computeReflectionLoss(const IMaterial *incidentMaterial, const IMaterial *refractiveMaterial, double angle) const;
    virtual double computeObjectLoss(const IPhysicalObject *object, Hz frequency, const Coord& transmissionPosition, const Coord& receptionPosition) const;

    virtual void fireObstaclePenetrated(const IPhysicalObject *object, const Coord& intersection1, const Coord& intersection2, const Coord& normal1, const Coord& normal2) const;

  public:
    TracingObstacleLoss();

    virtual std::ostream& printToStream(std::ostream& stream, int level) const override;

    virtual void addListener(ITracingObstacleLossListener *listener) { listeners.push_back(listener); }
    virtual void removeListener(ITracingObstacleLossListener *listener) { listeners.erase(std::remove(listeners.begin(), listeners.end(), listener), listeners.end()); }

    virtual double computeObstacleLoss(Hz frequency, const Coord& transmissionPosition, const Coord& receptionPosition) const override;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_TRACINGOBSTACLELOSS_H

