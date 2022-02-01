//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.base;

//
// This is a base module for IEEE 802.11 association visualizer simple modules.
// It keeps track of wireless interface associations by subscribing to a number
// of signals at the provided subscription module.
//
// This module subscribes for l2Associated, l2Disassociated,
// l2ApAssociated, l2ApDisassociated on the module determined by the
// visualizationSubjectModule parameter.
//
// @see ~Ieee80211CanvasVisualizer, ~Ieee80211OsgVisualizer, ~IIeee80211Visualizer, ~VisualizerBase
//
simple Ieee80211VisualizerBase extends VisualizerBase
{
    parameters:
        string networkNodeVisualizerModule = default("^.networkNodeVisualizer"); // module path to the network node visualizer module, refers to a sibling submodule by default

        bool displayAssociations = default(false); // display association icon and SSID, disabled by default

        string nodeFilter @mutable = default("*"); // which nodes are considered, matches all nodes by default
        string interfaceFilter @mutable = default("*"); // which interfaces are considered, matches all interfaces by default

        double minPower @unit(dBm) = default(-85dBm); // minimum expected signal power
        double maxPower @unit(dBm) = default(-50dBm); // maximum expeceted signal power

        string icons = default("misc/signal_power_0 misc/signal_power_1 misc/signal_power_2 misc/signal_power_3"); // wireless network visualization icon for different signal powers
        string iconColor @enum("light", "dark") = default("dark"); // wireless network icon color is a list of colors, a set of dark colors by default
        string labelFont = default("<default>"); // label font, automatic by default
        string labelColor = default("black"); // label color, black by default
        string placementHint = default("top"); // annotation placement hint, space separated list of any, top, bottom, left, right, topLeft, topCenter, topRight, etc.
        double placementPriority = default(0); // determines the order of annotation positioning

        @class(Ieee80211VisualizerBase);
}

