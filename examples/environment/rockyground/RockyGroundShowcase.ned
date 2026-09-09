//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.examples.environment.rockyground;

import inet.common.geometry.common.SimpleGeographicCoordinateSystem;
import inet.environment.common.PhysicalEnvironment;

//
// Minimal RockyGround smoke test: a physical environment whose ground is a
// RockyGround (real geospatial elevation via Rocky), anchored to a geographic
// origin by a SimpleGeographicCoordinateSystem. RockyGround logs the ground
// elevation it samples at a few scene points during initialization.
//
network RockyGroundShowcase
{
    submodules:
        coordinateSystem: SimpleGeographicCoordinateSystem;
        physicalEnvironment: PhysicalEnvironment;
}
