//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.base;

//
// This is a base module for physical layer link visualizer simple modules. It
// visualizes active communication links at the physical layer, showing successful
// packet transmissions between radio devices.
//
// By default, physical links are not displayed. To enable visualization, set the
// displayLinks parameter to true in the derived visualizer module and use the
// filters to select which nodes, interfaces, and packets to monitor.
//
// The visualizer shows physical layer links as dotted olive-colored arrows between
// network nodes, with optional labels showing packet information. Links become
// active when a packet successfully travels from one node's physical layer to
// another's, and fade out after a configurable amount of time if no new packets
// are transmitted.
//
// This module subscribes to the `packetReceivedFromUpper` and `packetSentToUpper` events
// on the module determined by the `visualizationSubjectModule` parameter. When a packet
// is handed down to a physical layer (packetReceivedFromUpper), the visualizer stores
// its ID. When that same packet ID is later detected being passed up from another
// physical layer (packetSentToUpper), the visualizer creates or refreshes a link
// between the source and destination nodes.
//
// Physical link visualization is particularly useful for understanding wireless
// communication patterns, analyzing which nodes are actively communicating at the
// physical layer, and debugging radio connectivity issues.
//
// @see ~PhysicalLinkCanvasVisualizer, ~PhysicalLinkOsgVisualizer, ~IPhysicalLinkVisualizer, ~VisualizerBase
//
simple PhysicalLinkVisualizerBase extends LinkVisualizerBase
{
    parameters:
        lineColor = default("olive");
        lineStyle = default("dotted");
        labelColor = default("olive");
        @class(PhysicalLinkVisualizerBase);
}
