//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.base;

//
// This is a base module for data link layer link visualizer simple modules. It
// keeps track of active links in a network. A link between two network node is
// considered active if a packet is sent recently between their link layers.
// Active links are not displayed by default, see the module parameters for how
// to enable the visualization. The actual visualization of active links is done
// in derived modules.
//
// This module subscribes for packetReceivedFromUpper and packetSentToUpper on
// the module determined by the visualizationSubjectModule parameter. If the visualizer
// receives a packetReceivedFromUpper from a MAC protocol module, then it stores
// the corresponding packet id. If later on the visualizer receives a
// packetSentToUpper from another MAC protocol module with the same packet id,
// then it adds an active link between the source and the destination nodes.
// After a certain amount of time the link becomes inactive unless it is
// reinforced by another packet.
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

