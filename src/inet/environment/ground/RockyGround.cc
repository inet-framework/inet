//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/environment/ground/RockyGround.h"

#if defined(WITH_ROCKY)

#include <cmath>
#include <string>

#include <rocky/GeoPoint.h>
#include <rocky/IOTypes.h>
#include <rocky/SRS.h>

#include "inet/common/InitStages.h"

namespace inet {

namespace physicalenvironment {

Define_Module(RockyGround);

int RockyGround::numInitStages() const
{
    return NUM_INIT_STAGES;
}

void RockyGround::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        coordinateSystem.reference(this, "coordinateSystemModule", true);
        outOfBoundsElevation = par("outOfBoundsElevation").doubleValue();

        // A Context runs GDALAllRegister()/OGRRegisterAll() in its constructor and
        // provides the IOOptions the sampler needs; keep the owner alive for our lifetime.
        contextOwner = rocky::ContextFactory::create();
        context = contextOwner.get();

        elevationLayer = rocky::GDALElevationLayer::create();
        elevationLayer->uri = rocky::URI(std::string(par("elevationFile").stringValue()));
        auto opened = elevationLayer->open(context->io);
        if (!opened.ok())
            throw cRuntimeError("RockyGround: cannot open elevation source '%s': %s",
                    par("elevationFile").stringValue(), opened.error().message.c_str());

        sampler.layer = elevationLayer;
        if (!sampler.ok())
            throw cRuntimeError("RockyGround: elevation sampler not ready (layer status: %s)",
                    elevationLayer->status().error().message.c_str());

        EV_INFO << "RockyGround: opened elevation source '" << par("elevationFile").stringValue()
                << "' (profile SRS " << elevationLayer->profile.srs().name() << ")\n";
    }
    else if (stage == INITSTAGE_LAST) {
        // Diagnostic: sample the ground through the full scene->geographic->sample->scene path
        // at a few scene points, once the coordinate system is initialized. Proves the physics
        // query works end to end and shows the terrain the radio models will see.
        for (const Coord& p : {Coord(0, 0, 0), Coord(100, 0, 0), Coord(0, 100, 0)}) {
            Coord ground = computeGroundProjection(p);
            EV_INFO << "RockyGround: ground elevation at scene (" << p.x << ", " << p.y << ") = " << ground.z << " m\n";
        }
    }
}

double RockyGround::sampleElevation(double latDeg, double lonDeg) const
{
    auto result = sampler.sample(rocky::GeoPoint(rocky::SRS::WGS84, lonDeg, latDeg, 0.0), context->io);
    if (result.ok()) {
        float height = result.value().height;
        if (height != rocky::NO_DATA_VALUE && !std::isnan(height))
            return height;
    }
    return outOfBoundsElevation;
}

Coord RockyGround::computeGroundProjection(const Coord& position) const
{
    auto geoCoord = coordinateSystem->computeGeographicCoordinate(position);
    double elevation = sampleElevation(geoCoord.latitude.get<deg>(), geoCoord.longitude.get<deg>());
    geoCoord.altitude = m(elevation);
    return coordinateSystem->computeSceneCoordinate(geoCoord);
}

Coord RockyGround::computeGroundNormal(const Coord& position) const
{
    // three samples (center + two offsets), cross product of the two edges — the
    // same construction as OsgEarthGround::computeGroundNormal.
    double distance = 1; // meters between the samples
    Coord A = computeGroundProjection(position);
    Coord B = computeGroundProjection(position + Coord(distance, 0, 0));
    Coord C = computeGroundProjection(position + Coord(0, distance, 0));
    Coord V1 = B - A;
    Coord V2 = C - A;
    Coord normal = V1 % V2;
    normal.normalize();
    return normal;
}

} // namespace physicalenvironment

} // namespace inet

#endif // defined(WITH_ROCKY)
