//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_POINTCLOUDGROUND_H
#define __INET_POINTCLOUDGROUND_H

#include "inet/common/Module.h"
#include "inet/common/geometry/common/Heightfield.h"
#include "inet/environment/contract/IGround.h"

namespace inet {

namespace physicalenvironment {

/**
 * Ground model whose surface is a digital surface model rasterized from a PLY
 * point cloud (e.g. a LIDAR scan) — the physics-side counterpart of the VSG
 * scene visualizer's sceneModel terrain. See PointCloudGround.ned for the
 * parameters (file, cell size, and the PLY-to-simulation-coordinate transform).
 */
class INET_API PointCloudGround : public IGround, public Module
{
  protected:
    Heightfield heightfield;
    double outOfBoundsElevation = NaN;

  protected:
    virtual void initialize(int stage) override;

  public:
    virtual Coord computeGroundProjection(const Coord& position) const override;
    virtual Coord computeGroundNormal(const Coord& position) const override;

    const Heightfield& getHeightfield() const { return heightfield; }
};

} // namespace physicalenvironment

} // namespace inet

#endif
