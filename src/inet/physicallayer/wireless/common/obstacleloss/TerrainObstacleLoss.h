//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TERRAINOBSTACLELOSS_H
#define __INET_TERRAINOBSTACLELOSS_H

#include <map>
#include <vector>

#include "inet/common/Module.h"
#include "inet/common/ModuleRefByPar.h"
#include "inet/common/geometry/common/Heightfield.h"
#include "inet/environment/contract/IPhysicalEnvironment.h"
#include "inet/mobility/contract/IMobility.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IObstacleLoss.h"

namespace inet {

namespace physicallayer {

/**
 * Minimal terrain occlusion: blocks a transmission entirely (loss factor 0)
 * when the terrain surface — as rasterized by the PointCloudGround ground
 * module — rises above the straight line between the transmitter and the
 * receiver; otherwise the transmission is unaffected (loss factor 1).
 * Endpoints are excluded from the test so antennas standing on the surface
 * do not occlude themselves. See TerrainObstacleLoss.ned.
 */
class INET_API TerrainObstacleLoss : public Module, public IObstacleLoss
{
  protected:
    ModuleRefByPar<physicalenvironment::IPhysicalEnvironment> physicalEnvironment;
    mutable const Heightfield *heightfield = nullptr; // resolved lazily: the ground module may initialize after this one
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

  public:
    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    virtual double computeObstacleLoss(Hz frequency, const Coord& transmissionPosition, const Coord& receptionPosition) const override;
};

} // namespace physicallayer

} // namespace inet

#endif
