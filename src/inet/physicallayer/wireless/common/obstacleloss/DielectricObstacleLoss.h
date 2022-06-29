//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_DIELECTRICOBSTACLELOSS_H
#define __INET_DIELECTRICOBSTACLELOSS_H

#include <algorithm>

#include "inet/common/IVisitor.h"
#include "inet/common/ModuleRefByPar.h"
#include "inet/environment/contract/IPhysicalEnvironment.h"
#include "inet/physicallayer/wireless/common/base/packetlevel/TracingObstacleLossBase.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadioMedium.h"

namespace inet {

namespace physicallayer {

/**
 * This class computes obstacle loss based on the actual straight path that the
 * radio signal travels from the transmitter to the receiver. The total loss is
 * the combination of the dielectric losses in the intersected obstacles and the
 * reflection losses of the penetrated faces along this path.
 */
class INET_API DielectricObstacleLoss : public TracingObstacleLossBase
{
  protected:
    class INET_API TotalObstacleLossComputation : public IVisitor {
      protected:
        mutable double totalLoss;
        const DielectricObstacleLoss *obstacleLoss;
        const Hz frequency;
        const Coord transmissionPosition;
        const Coord receptionPosition;

      public:
        TotalObstacleLossComputation(const DielectricObstacleLoss *obstacleLoss, Hz frequency, const Coord& transmissionPosition, const Coord& receptionPosition);
        void visit(const cObject *object) const override;
        double getTotalLoss() const { return totalLoss; }
    };

  protected:
    /** @name Parameters */
    //@{
    bool enableDielectricLoss = false;
    bool enableReflectionLoss = false;
    /**
     * The radio medium where the radio signal propagation takes place.
     */
    IRadioMedium *medium;
    /**
     * The physical environment that provides to obstacles.
     */
    ModuleRefByPar<physicalenvironment::IPhysicalEnvironment> physicalEnvironment;
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

    virtual double computeDielectricLoss(const physicalenvironment::IMaterial *material, Hz frequency, m distance) const;
    virtual double computeReflectionLoss(const physicalenvironment::IMaterial *incidentMaterial, const physicalenvironment::IMaterial *refractiveMaterial, double angle) const;
    virtual double computeObjectLoss(const physicalenvironment::IPhysicalObject *object, Hz frequency, const Coord& transmissionPosition, const Coord& receptionPosition) const;

  public:
    DielectricObstacleLoss();

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    virtual double computeObstacleLoss(Hz frequency, const Coord& transmissionPosition, const Coord& receptionPosition) const override;
};

} // namespace physicallayer

} // namespace inet

#endif

