//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.mobility.geographic;

import inet.common.SimpleModule;

//
// Controls a set of satellite nodes propagated from one or more TLE files. It is
// the authority for the absolute UTC `epoch` (mapped to simulation time 0), which
// it distributes to the ~SatelliteMobility of each satellite it creates.
//
// At simulation start (or at the times given by the optional `schedule`) it
// dynamically creates a node of type `nodeType` per satellite, as elements of the
// `nodeName` submodule vector of the network, and injects the TLE selection and
// epoch into each node's `mobility` submodule. The node's mobility must be a
// ~SatelliteMobility (e.g. set `**.<nodeName>[*].mobility.typename` accordingly).
//
// The controller keeps a registry of the live satellite modules and emits the
// `satelliteInserted` and `satelliteRemoved` signals on each lifecycle event, so a
// visualizer can enumerate and track the satellites.
//
// The optional `schedule` XML may contain `<satellite>` elements with `name` or
// `index`, and `insertTime`/`removeTime` attributes, e.g.:
//   <schedule>
//     <satellite name="ISS (ZARYA)" insertTime="0s" removeTime="3600s"/>
//   </schedule>
//
simple SatelliteController extends SimpleModule
{
    parameters:
        @class(SatelliteController);
        @display("i=block/control");
        @signal[satelliteInserted](type=cModule);
        @signal[satelliteRemoved](type=cModule);
        string tleFiles; // One or more TLE file paths, separated by whitespace, ';' or ','
        string epoch; // Absolute UTC time (ISO-8601, e.g. "2026-06-02T00:00:00Z") mapped to simulation time 0
        string nodeType = default("inet.node.inet.StandardHost"); // NED type of the node created per satellite
        string nodeName = default("satellite"); // Name of the submodule vector holding the created nodes
        string coordinateSystemModule = default(""); // Geographic coordinate system path injected into the satellite mobilities (optional)
        double insertionTime @unit(s) = default(0s); // Default time at which satellites are inserted
        xml schedule = default(xml("<schedule/>")); // Optional per-satellite insertion/removal schedule
        // scene layout and icon of the created node modules
        string nodeIcon = default("misc/satellite2"); // icon image for created node modules (display-string 'i' tag); empty keeps the node type's own icon
        double nodeIconRowX = default(40); // x position (pixels) of the first created node icon
        double nodeIconRowY = default(400); // y position (pixels) of the created node icon row
        double nodeIconSpacing = default(80); // horizontal spacing (pixels) between created node icons
}

