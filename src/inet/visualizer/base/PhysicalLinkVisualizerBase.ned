//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.base;

//
// This is a base module for physical layer link visualizer simple modules. It
// keeps track of active links in a network. A link between two network node is
// considered active if a packet is sent recently between their physical layers.
// Active links are not displayed by default, see the module parameters for how
// to enable the visualization. The actual visualization of active links is done
// in derived modules.
//
// This module subscribes for packetReceivedFromUpper and packetSentToUpper on
// the module determined by the visualizationSubjectModule parameter. If the visualizer
// receives a packetReceivedFromUpper from a physical layer module, then it
// stores the corresponding packet id. If later on the visualizer receives a
// packetSentToUpper from another physical layer module with the same packet
// id, then it adds an active link between the source and the destination nodes.
// After a certain amount of time the link becomes inactive unless it is
// reinforced by another packet.
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

