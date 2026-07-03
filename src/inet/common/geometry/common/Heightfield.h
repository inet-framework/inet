//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_HEIGHTFIELD_H
#define __INET_HEIGHTFIELD_H

#include <vector>

#include "inet/common/geometry/common/Coord.h"

namespace inet {

/**
 * A regular 2.5D elevation grid (digital surface model) rasterized from a
 * point cloud: each grid cell stores the maximum z of the points falling into
 * it (DSM semantics — building tops and canopy are preserved), with small
 * holes filled by bounded neighbor-averaging passes. Provides O(1) bilinear
 * elevation lookup, surface normals, and elevation profiles along a segment
 * (for line-of-sight / diffraction models).
 *
 * Elevation queries outside the data extent (or in unfilled holes) return NaN;
 * callers decide how to substitute. Rasterization uses a max-reduce, so the
 * result is independent of point order (deterministic across platforms).
 */
class INET_API Heightfield
{
  protected:
    double minX = 0, minY = 0;
    double cellSize = 0;
    int numCellsX = 0, numCellsY = 0;
    std::vector<float> cells; // row-major [y * numCellsX + x], NaN = no data

  protected:
    float getCell(int ix, int iy) const { return cells[iy * numCellsX + ix]; }

  public:
    /**
     * Rasterizes the points into a grid. A cellSize <= 0 selects an automatic
     * cell size approximating the mean point spacing, sqrt(area / count).
     * A positive despikeThreshold clamps isolated single-cell spikes that
     * exceed ALL their neighbors by more than the threshold (raw LIDAR tiles
     * commonly contain stray high returns — birds, atmospheric noise — which
     * max-z rasterization would otherwise keep as phantom needles); genuine
     * structures survive because their edge cells have same-height neighbors.
     * Throws cRuntimeError if the grid would exceed maxCells (the message
     * suggests a coarser cellSize) or if the input is empty/degenerate.
     */
    void buildFromPoints(const std::vector<double>& xs, const std::vector<double>& ys, const std::vector<double>& zs,
            double cellSize = 0, int64_t maxCells = 67108864, int holeFillPasses = 8, double despikeThreshold = 0);

    bool isValid() const { return numCellsX > 0 && numCellsY > 0; }
    double getCellSize() const { return cellSize; }
    int getNumCellsX() const { return numCellsX; }
    int getNumCellsY() const { return numCellsY; }
    double getMinX() const { return minX; }
    double getMinY() const { return minY; }
    double getMaxX() const { return minX + numCellsX * cellSize; }
    double getMaxY() const { return minY + numCellsY * cellSize; }

    /**
     * Bilinearly interpolated elevation at (x, y) between cell centers;
     * NaN outside the extent or where no data survived hole filling.
     */
    double getElevation(double x, double y) const;

    /**
     * Upward unit surface normal from central differences of the elevation;
     * (NaN, NaN, NaN) where the elevation is undetermined.
     */
    Coord getNormal(double x, double y) const;

    /**
     * Elevations sampled every 'step' meters along the segment from a to b
     * (endpoints included; only x/y of the inputs are used). step must be > 0.
     */
    std::vector<double> computeProfile(const Coord& a, const Coord& b, double step) const;

    /**
     * The center of the highest cell and its elevation (e.g. the tallest
     * building top) — useful for placing nodes relative to landmarks.
     */
    Coord getPeakLocation() const;
};

} // namespace inet

#endif
