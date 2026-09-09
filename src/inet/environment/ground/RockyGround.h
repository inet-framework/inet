//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ROCKYGROUND_H
#define __INET_ROCKYGROUND_H

#include "inet/environment/contract/IGround.h"

#if defined(WITH_ROCKY)

#include <memory>

#include <rocky/Context.h>
#include <rocky/ElevationSampler.h>
#include <rocky/GDALElevationLayer.h>

#include "inet/common/Module.h"
#include "inet/common/ModuleRefByPar.h"
#include "inet/common/geometry/common/GeographicCoordinateSystem.h"

namespace inet {

namespace physicalenvironment {

/**
 * Ground surface backed by Rocky (the VSG geospatial SDK, osgEarth's successor):
 * elevation comes from a Rocky elevation layer (e.g. a local GeoTIFF DEM via
 * GDAL) sampled with rocky::ElevationSampler. Scene positions are converted to
 * geographic coordinates via an IGeographicCoordinateSystem, sampled, and
 * converted back — the same pattern as ~OsgEarthGround, but backend-neutral of
 * OSG and headless (no rendering needed). See RockyGround.ned.
 */
class INET_API RockyGround : public IGround, public Module
{
  protected:
    // ContextSingleton owns the ContextImpl; Context is a raw ContextImpl* into it,
    // so the singleton must outlive every use (kept as a member for the module lifetime).
    rocky::ContextSingleton contextOwner;
    rocky::Context context = nullptr;
    std::shared_ptr<rocky::GDALElevationLayer> elevationLayer;
    mutable rocky::ElevationSampler sampler;
    ModuleRefByPar<IGeographicCoordinateSystem> coordinateSystem;
    double outOfBoundsElevation = 0;

    virtual int numInitStages() const override;
    virtual void initialize(int stage) override;
    // Ground height (meters) at a geographic point, or outOfBoundsElevation where the layer has no data.
    double sampleElevation(double latDeg, double lonDeg) const;

  public:
    virtual Coord computeGroundProjection(const Coord& position) const override;
    virtual Coord computeGroundNormal(const Coord& position) const override;
};

} // namespace physicalenvironment

} // namespace inet

#endif // defined(WITH_ROCKY)

#endif
