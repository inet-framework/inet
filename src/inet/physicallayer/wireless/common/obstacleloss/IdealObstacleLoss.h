//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IDEALOBSTACLELOSS_H
#define __INET_IDEALOBSTACLELOSS_H

#include "inet/common/IVisitor.h"
#include "inet/common/ModuleRefByPar.h"
#include "inet/common/figures/TrailFigure.h"
#include "inet/environment/contract/IPhysicalEnvironment.h"
#include "inet/physicallayer/wireless/common/base/packetlevel/TracingObstacleLossBase.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadioMedium.h"

namespace inet {

namespace physicallayer {

class INET_API IdealObstacleLoss : public TracingObstacleLossBase
{
  protected:
    class INET_API TotalObstacleLossComputation : public IVisitor {
      protected:
        const IdealObstacleLoss *obstacleLoss = nullptr;
        const Coord transmissionPosition;
        const Coord receptionPosition;
        mutable bool isObstacleFound_ = false;

      public:
        TotalObstacleLossComputation(const IdealObstacleLoss *obstacleLoss, const Coord& transmissionPosition, const Coord& receptionPosition);
        void visit(const cObject *object) const override;
        bool isObstacleFound() const { return isObstacleFound_; }
    };

    /** @name Parameters */
    //@{
    /**
     * The radio medium where the radio signal propagation takes place.
     */
    IRadioMedium *medium = nullptr;
    /**
     * The physical environment that provides to obstacles.
     */
    ModuleRefByPar<physicalenvironment::IPhysicalEnvironment> physicalEnvironment;
    //@}

  protected:
    virtual void initialize(int stage) override;

    virtual bool isObstacle(const physicalenvironment::IPhysicalObject *object, const Coord& transmissionPosition, const Coord& receptionPosition) const;

  public:
    IdealObstacleLoss();
    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    virtual double computeObstacleLoss(Hz frequency, const Coord& transmissionPosition, const Coord& receptionPosition) const override;
};

} // namespace physicallayer

} // namespace inet

#endif

