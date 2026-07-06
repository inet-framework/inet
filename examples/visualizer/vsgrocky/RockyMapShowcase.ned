//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.examples.visualizer.vsgrocky;

import inet.common.geometry.common.SimpleGeographicCoordinateSystem;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.WirelessHost;
import inet.physicallayer.wireless.common.contract.packetlevel.IRadioMedium;
import inet.visualizer.vsg.integrated.IntegratedVsgVisualizer;

//
// A few wireless hosts standing on a real geospatial map rendered by Rocky in
// the VSG 3D view (SceneRockyVisualizer). The map is anchored at the geographic
// origin declared by the SimpleGeographicCoordinateSystem.
//
network RockyMapShowcase
{
    submodules:
        visualizer: IntegratedVsgVisualizer;
        coordinateSystem: SimpleGeographicCoordinateSystem;
        configurator: Ipv4NetworkConfigurator;
        radioMedium: <default("Ieee80211ScalarRadioMedium")> like IRadioMedium;
        host1: WirelessHost;
        host2: WirelessHost;
        host3: WirelessHost;
}
