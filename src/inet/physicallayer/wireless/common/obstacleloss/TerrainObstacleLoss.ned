//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.obstacleloss;

import inet.physicallayer.wireless.common.base.packetlevel.TracingObstacleLossBase;

//
// Terrain occlusion model driven by the physical environment's ground module,
// which must be a ~PointCloudGround (e.g. a LIDAR scan, so buildings present
// in the scan occlude links passing behind them). The straight line between
// the transmitter and the receiver is sampled at the ground heightfield's
// cell size by default; endpoints are excluded so antennas standing on the
// surface do not occlude themselves.
//
// Two modes:
// - "los" (default): binary — the transmission is blocked entirely (loss
//   factor 0) when the terrain surface rises above the direct line, and is
//   unaffected (loss factor 1) otherwise.
// - "fresnel": graded — the worst first-Fresnel-zone clearance along the path
//   is mapped through the ITU-R P.526 single knife-edge diffraction curve
//   J(v): no loss while the zone is clear by more than ~0.55 of its radius,
//   about 6 dB with the terrain right at the direct ray, deepening smoothly
//   (and frequency-dependently) as the ray sinks below the terrain. Links
//   thus degrade progressively instead of dropping at an invisible boundary.
//
// Emits obstaclePenetrated events at the critical terrain point (with no
// associated physical object, world coordinates), so tracing obstacle loss
// visualizers can display where links are obstructed.
//
// Multi-edge (Deygout) diffraction is future work.
//
module TerrainObstacleLoss extends TracingObstacleLossBase
{
    parameters:
        string mode @enum("los", "fresnel") = default("los"); // binary line-of-sight blocking, or graded Fresnel/knife-edge attenuation
        double sampleStep @unit(m) = default(0m); // spacing of the line-of-sight samples; 0 = the ground heightfield's cell size
        bool logLinkEvents = default(true); // log line-of-sight transitions per node pair (established / lost / re-established / cannot be established)
        @class(TerrainObstacleLoss);
}
