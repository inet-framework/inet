//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PLYPOINTCLOUDREADER_H
#define __INET_PLYPOINTCLOUDREADER_H

#include <string>
#include <vector>

#include "inet/common/INETDefs.h"

namespace inet {

/**
 * A point cloud loaded from a PLY file (e.g. a LIDAR scan). Coordinates are
 * kept exactly as stored in the file (no recentering or scaling); consumers
 * apply their own transform. Color channels, when present, are normalized to
 * [0,1] (8-bit channels divided by 255, wider/float channels taken as-is).
 */
class INET_API PlyPointCloud
{
  public:
    std::vector<double> xs;
    std::vector<double> ys;
    std::vector<double> zs;
    std::vector<double> rs; // empty unless hasRGB
    std::vector<double> gs; // empty unless hasRGB
    std::vector<double> bs; // empty unless hasRGB
    bool hasRGB = false;
    double minX = 0, maxX = 0;
    double minY = 0, maxY = 0;
    double minZ = 0, maxZ = 0;

    int getNumPoints() const { return (int)xs.size(); }
};

/**
 * Reads PLY point clouds: ascii and binary_little_endian formats, vertex
 * properties located by name (x/y/z required, red/green/blue or r/g/b
 * optional) with arbitrary scalar types and property order. List properties
 * (e.g. face vertex-index lists) are skipped — only points are read.
 * Throws cRuntimeError with a descriptive message on malformed input.
 *
 * Extracted from the VSG scene visualizer's terrain loader so that both the
 * visualization and the physical models (ground/obstacle) read LIDAR data
 * through one implementation.
 */
class INET_API PlyPointCloudReader
{
  public:
    static PlyPointCloud read(const std::string& path);
};

} // namespace inet

#endif
