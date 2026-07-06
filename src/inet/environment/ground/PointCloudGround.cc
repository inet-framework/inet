//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/environment/ground/PointCloudGround.h"

#include <cmath>
#include <ctime>

#include "inet/common/geometry/common/PlyPointCloudReader.h"

namespace inet {

namespace physicalenvironment {

Define_Module(PointCloudGround);

void PointCloudGround::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        const char *file = par("file");
        double cellSize = par("cellSize");
        int64_t maxCells = par("maxCells");
        outOfBoundsElevation = par("outOfBoundsElevation");
        const char *transformMode = par("transformMode");

        long startTime = clock();
        PlyPointCloud cloud = PlyPointCloudReader::read(file);

        // reduce every mode to point' = point * scale + offset (per axis)
        double scale = 1, offsetX = 0, offsetY = 0, offsetZ = 0;
        if (!strcmp(transformMode, "metric")) {
            // translate only: 1 PLY unit = 1 simulation meter (physics-correct)
            scale = 1;
            offsetX = par("originX").doubleValue() - cloud.minX;
            offsetY = par("originY").doubleValue() - cloud.minY;
            offsetZ = par("baseElevation").doubleValue() - cloud.minZ;
        }
        else if (!strcmp(transformMode, "fit")) {
            // reproduce the VSG scene visualizer's display transform: recenter the
            // cloud's bbox onto the scene center and aspect-fit it into the scene
            // footprint; fitMin/fitMax must match the visualizer's scene bounds
            double fitMinX = par("fitMinX"), fitMinY = par("fitMinY"), fitMinZ = par("fitMinZ");
            double fitMaxX = par("fitMaxX"), fitMaxY = par("fitMaxY");
            if (std::isnan(fitMinX) || std::isnan(fitMinY) || std::isnan(fitMinZ) || std::isnan(fitMaxX) || std::isnan(fitMaxY))
                throw cRuntimeError("transformMode=\"fit\" requires the fitMinX/fitMinY/fitMinZ/fitMaxX/fitMaxY parameters (set them to the scene visualizer's scene bounds)");
            double plyW = cloud.maxX - cloud.minX, plyH = cloud.maxY - cloud.minY;
            double sceneW = fitMaxX - fitMinX, sceneH = fitMaxY - fitMinY;
            scale = (plyW > 0 && plyH > 0 && sceneW > 0 && sceneH > 0) ? std::min(sceneW / plyW, sceneH / plyH) : 1.0;
            double cx = (cloud.minX + cloud.maxX) / 2, cy = (cloud.minY + cloud.maxY) / 2;
            offsetX = (fitMinX + fitMaxX) / 2 - cx * scale;
            offsetY = (fitMinY + fitMaxY) / 2 - cy * scale;
            offsetZ = fitMinZ - cloud.minZ * scale;
            EV_WARN << "PointCloudGround: transformMode=\"fit\" scales the terrain by " << scale
                    << " to fit the scene; physical distances over the terrain are scaled accordingly — use \"metric\" for physically exact studies\n";
        }
        else if (!strcmp(transformMode, "manual")) {
            scale = par("scale");
            offsetX = par("offsetX");
            offsetY = par("offsetY");
            offsetZ = par("offsetZ");
        }
        else
            throw cRuntimeError("Unknown transformMode '%s' (expected \"metric\", \"fit\" or \"manual\")", transformMode);

        std::vector<double> xs(cloud.getNumPoints()), ys(cloud.getNumPoints()), zs(cloud.getNumPoints());
        for (int i = 0; i < cloud.getNumPoints(); i++) {
            xs[i] = cloud.xs[i] * scale + offsetX;
            ys[i] = cloud.ys[i] * scale + offsetY;
            zs[i] = cloud.zs[i] * scale + offsetZ;
        }
        heightfield.buildFromPoints(xs, ys, zs, cellSize, maxCells, 8, par("despikeThreshold"));

        double elapsed = (double)(clock() - startTime) / CLOCKS_PER_SEC;
        Coord peak = heightfield.getPeakLocation();
        EV_INFO << "PointCloudGround: loaded " << cloud.getNumPoints() << " points from '" << file
                << "', rasterized into a " << heightfield.getNumCellsX() << "x" << heightfield.getNumCellsY()
                << " heightfield (cell size " << heightfield.getCellSize() << " m, transformMode " << transformMode
                << ", scale " << scale << ") in " << elapsed << " s; extent x=[" << heightfield.getMinX() << ", " << heightfield.getMaxX()
                << "] y=[" << heightfield.getMinY() << ", " << heightfield.getMaxY()
                << "], peak " << peak.z << " m at (" << peak.x << ", " << peak.y << ")\n";
    }
}

Coord PointCloudGround::computeGroundProjection(const Coord& position) const
{
    double elevation = heightfield.getElevation(position.x, position.y);
    if (std::isnan(elevation))
        elevation = outOfBoundsElevation;
    return Coord(position.x, position.y, elevation);
}

Coord PointCloudGround::computeGroundNormal(const Coord& position) const
{
    return heightfield.getNormal(position.x, position.y);
}

} // namespace physicalenvironment

} // namespace inet
