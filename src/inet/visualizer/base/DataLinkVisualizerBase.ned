//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.base;

//
// This is a base module for data link layer link visualizer simple modules. It
// visualizes active communication links at the data link (MAC) layer, showing
// successful frame transmissions between network nodes.
//
// By default, data link layer links are not displayed. To enable visualization,
// set the displayLinks parameter to true in the derived visualizer module and
// use the filters to select which nodes, interfaces, and packets to monitor.
//
// The visualizer shows data link layer connections as dark cyan arrows between
// network nodes, with optional labels showing packet information. Links become
// active when a frame successfully travels from one node's MAC layer to another's,
// and fade out after a configurable amount of time if no new frames are transmitted.
//
// This module subscribes for `packetReceivedFromUpper` and `packetSentToUpper` signals
// on the module determined by the `visualizationSubjectModule` parameter. When a packet
// is handed down to a MAC protocol module (packetReceivedFromUpper), the visualizer
// stores its ID. When that same packet ID is later detected being passed up from
// another MAC protocol module (packetSentToUpper), the visualizer creates or
// refreshes a link between the source and destination nodes.
//
// Data link visualization is particularly useful for understanding MAC layer
// communication patterns, analyzing which nodes are actively exchanging frames,
// debugging MAC protocol issues, and observing the effects of wireless interference
// on successful frame delivery.
//
// @see ~DataLinkCanvasVisualizer, ~DataLinkOsgVisualizer, ~IDataLinkVisualizer, ~VisualizerBase
//
simple DataLinkVisualizerBase extends LinkVisualizerBase
{
    parameters:
        lineColor = default("darkcyan");
        labelColor = default("darkcyan");
        @class(DataLinkVisualizerBase);
}
