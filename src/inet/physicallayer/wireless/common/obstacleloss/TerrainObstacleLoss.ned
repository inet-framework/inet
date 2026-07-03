//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.obstacleloss;

import inet.common.Module;
import inet.physicallayer.wireless.common.contract.packetlevel.IObstacleLoss;

//
// Minimal terrain occlusion model: a transmission is blocked entirely (loss
// factor 0) when the terrain surface rises above the straight line between
// the transmitter and the receiver, and is unaffected (loss factor 1)
// otherwise. The surface comes from the physical environment's ground
// module, which must be a ~PointCloudGround (e.g. a LIDAR scan, so buildings
// present in the scan occlude links passing behind them). The line is sampled
// at the ground heightfield's cell size by default; endpoints are excluded so
// antennas standing on the surface do not occlude themselves.
//
// This is the binary "los" mode of the planned terrain occlusion support;
// graded Fresnel-zone attenuation and knife-edge diffraction are future work.
//
module TerrainObstacleLoss extends Module like IObstacleLoss
{
    parameters:
        string physicalEnvironmentModule = default("physicalEnvironment"); // the physical environment whose ground provides the terrain
        double sampleStep @unit(m) = default(0m); // spacing of the line-of-sight samples; 0 = the ground heightfield's cell size
        bool logLinkEvents = default(true); // log line-of-sight transitions per node pair (established / lost / re-established / cannot be established)
        @class(TerrainObstacleLoss);
        @display("i=block/control");
}
