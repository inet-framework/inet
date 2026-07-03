//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.environment.ground;

import inet.common.Module;
import inet.environment.contract.IGround;

//
// Models the ground surface with a digital surface model rasterized from a
// PLY point cloud, e.g. a LIDAR scan (the same kind of file the VSG scene
// visualizer's sceneModel parameter displays). The points are binned into a
// regular grid keeping the maximum elevation per cell (so building tops and
// canopy are preserved), small gaps are filled from neighboring cells, and
// elevation queries interpolate bilinearly between cells. This makes
// ground-aware radio models (e.g. ~TwoRayGroundReflection) use the real
// terrain elevation under each node.
//
// The transformMode parameter controls how PLY coordinates map to simulation
// coordinates:
//  - "metric" (default): translate only — 1 PLY unit = 1 simulation meter,
//    the cloud's bounding-box minimum is placed at (originX, originY) and its
//    lowest point at baseElevation. Physically exact; use this for studies.
//  - "fit": reproduces the VSG scene visualizer's display transform (recenter
//    + aspect-preserving fit into the scene footprint), so the physical
//    terrain matches what the visualizer shows. Requires fitMinX/fitMinY/
//    fitMinZ/fitMaxX/fitMaxY to be set to the visualizer's scene bounds.
//    Note: this scales distances — a warning is logged.
//  - "manual": explicit scale and offsets, point' = point * scale + offset.
//
// Elevation queries outside the data extent return outOfBoundsElevation
// (NaN by default, following the ~IGround contract); set it to a number to
// clamp instead.
//
module PointCloudGround extends Module like IGround
{
    parameters:
        string file; // PLY point-cloud file (resolved relative to the working directory)
        double cellSize @unit(m) = default(0m); // heightfield cell size; 0 = automatic (approximates the mean point spacing)
        int maxCells = default(67108864); // guard against accidental huge grids; raise it or coarsen cellSize for very large tiles
        double despikeThreshold @unit(m) = default(50m); // clamp isolated single-cell spikes exceeding all neighbors by more than this (stray LIDAR returns); 0 disables
        string transformMode = default("metric"); // "metric", "fit" or "manual" (see the module description)
        double originX @unit(m) = default(0m); // metric mode: simulation x of the cloud's bounding-box minimum
        double originY @unit(m) = default(0m); // metric mode: simulation y of the cloud's bounding-box minimum
        double baseElevation @unit(m) = default(0m); // metric mode: simulation z of the cloud's lowest point
        double fitMinX @unit(m) = default(nan m); // fit mode: the scene visualizer's sceneMinX
        double fitMinY @unit(m) = default(nan m); // fit mode: the scene visualizer's sceneMinY
        double fitMinZ @unit(m) = default(nan m); // fit mode: the scene visualizer's sceneMinZ
        double fitMaxX @unit(m) = default(nan m); // fit mode: the scene visualizer's sceneMaxX
        double fitMaxY @unit(m) = default(nan m); // fit mode: the scene visualizer's sceneMaxY
        double offsetX @unit(m) = default(0m); // manual mode: x offset
        double offsetY @unit(m) = default(0m); // manual mode: y offset
        double offsetZ @unit(m) = default(0m); // manual mode: z offset
        double scale = default(1); // manual mode: uniform scale factor
        double outOfBoundsElevation @unit(m) = default(nan m); // elevation reported outside the data extent
        @class(PointCloudGround);
}
