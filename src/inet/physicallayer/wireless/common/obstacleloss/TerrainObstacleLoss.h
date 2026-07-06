//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TERRAINOBSTACLELOSS_H
#define __INET_TERRAINOBSTACLELOSS_H

#include <map>
#include <vector>

#include "inet/common/ModuleRefByPar.h"
#include "inet/common/geometry/common/Heightfield.h"
#include "inet/environment/contract/IPhysicalEnvironment.h"
#include "inet/mobility/contract/IMobility.h"
#include "inet/physicallayer/wireless/common/base/packetlevel/TracingObstacleLossBase.h"

namespace inet {

namespace physicallayer {

/**
 * Terrain occlusion from the PointCloudGround heightfield. In "los" mode a
 * transmission is blocked entirely (loss factor 0) when the terrain rises
 * above the straight line between transmitter and receiver. In "fresnel" mode
 * the loss is graded: the worst first-Fresnel-zone clearance along the path is
 * mapped through the ITU-R P.526 single knife-edge curve J(v), so links
 * degrade smoothly as terrain approaches and then crosses the direct ray.
 * Emits ITracingObstacleLoss::obstaclePenetratedSignal at the critical
 * terrain point (with no associated physical object, world coordinates).
 * See TerrainObstacleLoss.ned.
 */
class INET_API TerrainObstacleLoss : public TracingObstacleLossBase
{
  protected:
    enum Mode { LOS, FRESNEL };

    ModuleRefByPar<physicalenvironment::IPhysicalEnvironment> physicalEnvironment;
    mutable const Heightfield *heightfield = nullptr; // resolved lazily: the ground module may initialize after this one
    Mode mode = LOS;
    double sampleStep = 0;
    bool logLinkEvents = false;

    // line-of-sight bookkeeping for logging: last known blocked/clear state per node pair
    mutable std::vector<std::pair<const cModule *, const IMobility *>> nodes; // network nodes with a mobility submodule (lazy)
    mutable std::map<std::pair<const cModule *, const cModule *>, bool> pairBlocked;

  protected:
    virtual void initialize(int stage) override;
    const Heightfield *getHeightfield() const;
    const cModule *findNodeAt(const Coord& position) const;
    void logLineOfSightChange(const Coord& transmissionPosition, const Coord& receptionPosition, bool blocked) const;
    void emitObstaclePenetrated(const Coord& rayPoint, const Coord& groundPoint, double lossFactor) const;

  public:
    /**
     * Single knife-edge diffraction loss J(v) in dB from ITU-R P.526 for the
     * dimensionless diffraction parameter v; 0 dB for v <= -0.78 (the curve
     * is continuous at the cutoff).
     */
    static double computeKnifeEdgeLoss(double v);

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    virtual double computeObstacleLoss(Hz frequency, const Coord& transmissionPosition, const Coord& receptionPosition) const override;
};

} // namespace physicallayer

} // namespace inet

#endif
