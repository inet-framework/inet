//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.contract;

//
// Interface allows configuring different ~IIeee80211RateVisualizer
// submodules from INI files. Such a visualizer displays a small bar chart above
// each IEEE 802.11 access point, one bar per station, with the bar height
// proportional to the PHY data rate the access point uses towards that station.
//
// @see ~Ieee80211RateCanvasVisualizer, ~Ieee80211RateVisualizer, ~Ieee80211RateVisualizerBase
//
moduleinterface IIeee80211RateVisualizer extends IVisualizer
{
    parameters:
        @display("i=block/app");
}
