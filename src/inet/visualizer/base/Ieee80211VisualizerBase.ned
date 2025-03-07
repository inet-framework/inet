//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.base;

//
// This is a base module for IEEE 802.11 association visualizer simple modules.
// It visualizes Wi-Fi associations between stations and access points, showing
// connection status and signal strength with appropriate icons.
//
// By default, associations are not displayed. To enable visualization, set the
// displayAssociations parameter to true and use the filters to select which
// nodes and interfaces to visualize.
//
// The visualizer shows:
// - Wi-Fi association status between stations and access points
// - Signal strength indicators with different icons based on received power
// - SSID labels for the wireless networks
//
// The visualizer dynamically updates when stations associate or disassociate
// with access points by subscribing to l2Associated, l2Disassociated,
// l2ApAssociated, and l2ApDisassociated signals on the module determined by the
// visualizationSubjectModule parameter.
//
// The actual rendering of associations is implemented in derived modules for
// specific user interfaces (canvas or OSG).
//
// @see ~Ieee80211CanvasVisualizer, ~Ieee80211OsgVisualizer, ~IIeee80211Visualizer, ~VisualizerBase
//
simple Ieee80211VisualizerBase extends VisualizerBase
{
    parameters:
        string networkNodeVisualizerModule = default("^.networkNodeVisualizer"); // Module path to the network node visualizer module, refers to a sibling submodule by default

        bool displayAssociations = default(false); // Display association icon and SSID, disabled by default

        string nodeFilter @mutable = default("*"); // Which nodes are considered, matches all nodes by default
        string interfaceFilter @mutable = default("*"); // Which interfaces are considered, matches all interfaces by default

        double minPower @unit(dBm) = default(-85dBm); // Minimum expected signal power
        double maxPower @unit(dBm) = default(-50dBm); // Maximum expected signal power

        string icons = default("misc/signal_power_0 misc/signal_power_1 misc/signal_power_2 misc/signal_power_3"); // Wireless network visualization icon for different signal powers
        string iconColor = default("dark"); // Wireless network icon color is a list of colors or a color group name (e.g. dark, light), a set of dark colors by default
        string labelFont = default("<default>"); // Label font, automatic by default
        string labelColor = default("black"); // Label color, black by default
        string placementHint = default("top"); // Annotation placement hint, space-separated list of any, top, bottom, left, right, topLeft, topCenter, topRight, etc.
        double placementPriority = default(0); // Determines the order of annotation positioning

        @class(Ieee80211VisualizerBase);
}
