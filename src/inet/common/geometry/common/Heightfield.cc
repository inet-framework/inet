//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/geometry/common/Heightfield.h"

#include <cmath>
#include <limits>

namespace inet {

void Heightfield::buildFromPoints(const std::vector<double>& xs, const std::vector<double>& ys, const std::vector<double>& zs,
        double requestedCellSize, int64_t maxCells, int holeFillPasses, double despikeThreshold)
{
    size_t count = xs.size();
    if (count == 0 || ys.size() != count || zs.size() != count)
        throw cRuntimeError("Heightfield: empty or inconsistent point arrays (%zu/%zu/%zu points)", xs.size(), ys.size(), zs.size());

    double lo_x = xs[0], hi_x = xs[0], lo_y = ys[0], hi_y = ys[0];
    for (size_t i = 1; i < count; i++) {
        lo_x = std::min(lo_x, xs[i]); hi_x = std::max(hi_x, xs[i]);
        lo_y = std::min(lo_y, ys[i]); hi_y = std::max(hi_y, ys[i]);
    }
    double width = hi_x - lo_x, height = hi_y - lo_y;
    if (width <= 0 && height <= 0)
        throw cRuntimeError("Heightfield: degenerate point cloud (all points at the same x/y)");

    // auto cell size: approximate mean point spacing over the covered area
    double cs = requestedCellSize;
    if (cs <= 0)
        cs = std::sqrt(std::max(width, 1e-9) * std::max(height, 1e-9) / count);
    int64_t nx = std::max((int64_t)1, (int64_t)std::ceil(width / cs));
    int64_t ny = std::max((int64_t)1, (int64_t)std::ceil(height / cs));
    if (nx * ny > maxCells)
        throw cRuntimeError("Heightfield: grid of %lld x %lld cells exceeds the limit of %lld cells; use a larger cellSize (>= %g m) or raise maxCells",
                (long long)nx, (long long)ny, (long long)maxCells, std::sqrt((double)nx * ny / maxCells) * cs);

    minX = lo_x;
    minY = lo_y;
    cellSize = cs;
    numCellsX = (int)nx;
    numCellsY = (int)ny;
    cells.assign((size_t)(nx * ny), std::numeric_limits<float>::quiet_NaN());

    // rasterize: max z per cell (DSM semantics; order-independent, deterministic)
    for (size_t i = 0; i < count; i++) {
        int ix = std::min(numCellsX - 1, (int)((xs[i] - minX) / cellSize));
        int iy = std::min(numCellsY - 1, (int)((ys[i] - minY) / cellSize));
        float& cell = cells[iy * numCellsX + ix];
        float z = (float)zs[i];
        if (std::isnan(cell) || z > cell)
            cell = z;
    }

    // clamp isolated single-cell spikes (stray LIDAR returns) that exceed ALL
    // non-NaN neighbors by more than the threshold; real structures survive
    // because their edge cells have same-height neighbors
    if (despikeThreshold > 0) {
        std::vector<std::pair<size_t, float>> clamps;
        for (int iy = 0; iy < numCellsY; iy++) {
            for (int ix = 0; ix < numCellsX; ix++) {
                float v = getCell(ix, iy);
                if (std::isnan(v))
                    continue;
                float maxNeighbor = -std::numeric_limits<float>::infinity();
                int n = 0;
                for (int dy = -1; dy <= 1; dy++) {
                    for (int dx = -1; dx <= 1; dx++) {
                        if (dx == 0 && dy == 0) continue;
                        int jx = ix + dx, jy = iy + dy;
                        if (jx < 0 || jx >= numCellsX || jy < 0 || jy >= numCellsY) continue;
                        float w = getCell(jx, jy);
                        if (!std::isnan(w)) { maxNeighbor = std::max(maxNeighbor, w); n++; }
                    }
                }
                if (n >= 3 && v > maxNeighbor + despikeThreshold)
                    clamps.emplace_back((size_t)(iy * numCellsX + ix), maxNeighbor);
            }
        }
        for (auto& c : clamps)
            cells[c.first] = c.second;
    }

    // bounded hole filling: NaN cells take the average of their non-NaN 8-neighbors
    for (int pass = 0; pass < holeFillPasses; pass++) {
        std::vector<std::pair<size_t, float>> fills;
        for (int iy = 0; iy < numCellsY; iy++) {
            for (int ix = 0; ix < numCellsX; ix++) {
                if (!std::isnan(getCell(ix, iy)))
                    continue;
                double sum = 0;
                int n = 0;
                for (int dy = -1; dy <= 1; dy++) {
                    for (int dx = -1; dx <= 1; dx++) {
                        if (dx == 0 && dy == 0) continue;
                        int jx = ix + dx, jy = iy + dy;
                        if (jx < 0 || jx >= numCellsX || jy < 0 || jy >= numCellsY) continue;
                        float v = getCell(jx, jy);
                        if (!std::isnan(v)) { sum += v; n++; }
                    }
                }
                if (n >= 3) // require some support to avoid smearing data into large empty regions
                    fills.emplace_back((size_t)(iy * numCellsX + ix), (float)(sum / n));
            }
        }
        if (fills.empty())
            break;
        for (auto& f : fills)
            cells[f.first] = f.second;
    }
}

double Heightfield::getElevation(double x, double y) const
{
    if (!isValid())
        return NaN;
    if (x < minX || x > getMaxX() || y < minY || y > getMaxY())
        return NaN;
    // bilinear between cell centers, clamped at the borders
    double gx = (x - minX) / cellSize - 0.5;
    double gy = (y - minY) / cellSize - 0.5;
    int ix0 = (int)std::floor(gx), iy0 = (int)std::floor(gy);
    double fx = gx - ix0, fy = gy - iy0;
    ix0 = std::max(0, std::min(numCellsX - 1, ix0));
    iy0 = std::max(0, std::min(numCellsY - 1, iy0));
    int ix1 = std::min(numCellsX - 1, ix0 + 1);
    int iy1 = std::min(numCellsY - 1, iy0 + 1);
    float v00 = getCell(ix0, iy0), v10 = getCell(ix1, iy0);
    float v01 = getCell(ix0, iy1), v11 = getCell(ix1, iy1);
    if (std::isnan(v00) || std::isnan(v10) || std::isnan(v01) || std::isnan(v11)) {
        // fall back to the nearest cell's value (may itself be NaN in an unfilled hole)
        int ix = std::min(numCellsX - 1, (int)((x - minX) / cellSize));
        int iy = std::min(numCellsY - 1, (int)((y - minY) / cellSize));
        return getCell(ix, iy);
    }
    fx = std::max(0.0, std::min(1.0, fx));
    fy = std::max(0.0, std::min(1.0, fy));
    double v0 = v00 + (v10 - v00) * fx;
    double v1 = v01 + (v11 - v01) * fx;
    return v0 + (v1 - v0) * fy;
}

Coord Heightfield::getNormal(double x, double y) const
{
    double h = cellSize;
    double zxm = getElevation(x - h, y), zxp = getElevation(x + h, y);
    double zym = getElevation(x, y - h), zyp = getElevation(x, y + h);
    // fall back to one-sided differences at the borders
    double zc = NaN;
    if (std::isnan(zxm) || std::isnan(zxp) || std::isnan(zym) || std::isnan(zyp))
        zc = getElevation(x, y);
    double dx, dy;
    if (!std::isnan(zxm) && !std::isnan(zxp))
        dx = (zxp - zxm) / (2 * h);
    else if (!std::isnan(zxp) && !std::isnan(zc))
        dx = (zxp - zc) / h;
    else if (!std::isnan(zxm) && !std::isnan(zc))
        dx = (zc - zxm) / h;
    else
        return Coord(NaN, NaN, NaN);
    if (!std::isnan(zym) && !std::isnan(zyp))
        dy = (zyp - zym) / (2 * h);
    else if (!std::isnan(zyp) && !std::isnan(zc))
        dy = (zyp - zc) / h;
    else if (!std::isnan(zym) && !std::isnan(zc))
        dy = (zc - zym) / h;
    else
        return Coord(NaN, NaN, NaN);
    Coord normal(-dx, -dy, 1);
    normal.normalize();
    return normal;
}

Coord Heightfield::getPeakLocation() const
{
    int bestX = 0, bestY = 0;
    float best = -std::numeric_limits<float>::infinity();
    for (int iy = 0; iy < numCellsY; iy++)
        for (int ix = 0; ix < numCellsX; ix++) {
            float v = getCell(ix, iy);
            if (!std::isnan(v) && v > best) { best = v; bestX = ix; bestY = iy; }
        }
    return Coord(minX + (bestX + 0.5) * cellSize, minY + (bestY + 0.5) * cellSize, best);
}

std::vector<double> Heightfield::computeProfile(const Coord& a, const Coord& b, double step) const
{
    if (step <= 0)
        throw cRuntimeError("Heightfield::computeProfile: step must be positive (got %g)", step);
    double dx = b.x - a.x, dy = b.y - a.y;
    double distance = std::sqrt(dx * dx + dy * dy);
    int numSamples = std::max(2, (int)std::floor(distance / step) + 1);
    std::vector<double> profile(numSamples);
    for (int i = 0; i < numSamples; i++) {
        double t = (numSamples == 1) ? 0 : (double)i / (numSamples - 1);
        profile[i] = getElevation(a.x + dx * t, a.y + dy * t);
    }
    return profile;
}

} // namespace inet
