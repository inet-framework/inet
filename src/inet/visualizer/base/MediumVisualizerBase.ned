//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.base;

//
// This is a base module for medium visualizer simple modules. It keeps track
// of ongoing signal departures, ongoing signal arrivals, and propagating signals.
//
// It subscribes for signalSourceAdded, signalSourceRemoved, signalAdded,
// signalRemoved, signalDepartureStarted, signalDepartureEnded, signalArrivalStarted,
// signalArrivalEnded simulation signals at the module determined by the mediumModule
// parameter. The actual visualization of the medium is done in derived modules.
//
// @see ~MediumCanvasVisualizer, ~MediumOsgVisualizer, ~IMediumVisualizer, ~VisualizerBase
//
simple MediumVisualizerBase extends VisualizerBase
{
    parameters:
        string mediumModule = default("radioMedium"); // the medium module that is displayed, refers to a top level submodule by default
        string networkNodeVisualizerModule = default("^.networkNodeVisualizer"); // the network node visualizer module, refers to a sibling submodule by default

        string nodeFilter @mutable = default("*"); // which nodes are considered, matches all nodes by default
        string interfaceFilter @mutable = default("*"); // which interfaces are considered, matches all interfaces by default
        object packetFilter @mutable = default("*"); // which packets are considered, matches all packets by default

        bool displaySignals = default(false); // display signals propagating through the medium, disabled by default
        string signalColor = default("dark"); // signal color is a list of colors or a color group name (e.g. dark, light), a set of dark colors by default
        double signalPropagationAnimationSpeed @mutable = default(nan); // animation speed while the very beginning or end of a signal is propagating on the medium, value must be in the range (0, +inf) or NaN, the latter means automatic setting
        double signalPropagationAnimationTime @unit(s) = default(1s); // signal propagation animation duration when signalPropagationAnimationSpeed is unspecified
        double signalPropagationAdditionalTime @unit(s) = default(0s); // additional simulation time to animate signal propagation after leaving last receiver
        double signalTransmissionAnimationSpeed @mutable = default(nan); // animation speed while a signal is being transmitted on the medium, value must be in the range (0, +inf) or NaN, the latter means automatic setting
        double signalTransmissionAnimationTime @unit(s) = default(1s); // signal transmission animation duration when signalTransmissionAnimationSpeed is unspecified
        string signalAnimationSpeedChangeTimeMode @enum("realTime", "animationTime", "simulationTime") = default("realTime"); // specifies the time mode for animation speed changes between propagation and transmission animation
        double signalAnimationSpeedChangeTime @unit(s) = default(1s); // animation speed changes smoothly from propagationAnimationSpeed to transmissionAnimationSpeed in the specified real time

        bool displaySignalDepartures = default(false); // display an image where a signal departure is currently in progress, disabled by default
        string signalDepartureImage = default("misc/signal_departure.png"); // image for ongoing signal departures
        string signalDeparturePlacementHint = default("topCenter"); // annotation placement hint, space separated list of any, top, bottom, left, right, topLeft, topCenter, topRight, etc.
        double signalDeparturePlacementPriority = default(1); // determines the order of annotation positioning

        bool displaySignalArrivals = default(false); // display an image where a signal arrival is currently in progress, disabled by default
        string signalArrivalImage = default("misc/signal_arrival.png"); // image for ongoing signal arrivals
        string signalArrivalPlacementHint = default("topCenter"); // annotation placement hint, space separated list of any, top, bottom, left, right, topLeft, topCenter, topRight, etc.
        double signalArrivalPlacementPriority = default(-1); // determines the order of annotation positioning

        bool displayCommunicationRanges = default(false); // display communication ranges as circles around radios, enabled by default
        string communicationRangeLineColor = default("blue"); // communication range circle line color, blue by default
        string communicationRangeLineStyle = default("solid"); // communication range circle line style, solid by default
        double communicationRangeLineWidth = default(1); // communication range circle line width, 1 by default

        bool displayInterferenceRanges = default(false); // display interference ranges as circles around radios, disabled by default
        string interferenceRangeLineColor = default("gray"); // interference range circle color, gray by default
        string interferenceRangeLineStyle = default("solid"); // interference range circle line style, solid by default
        double interferenceRangeLineWidth = default(1); // interference range circle line width, 1 by default

        // parameters shared between power density maps, spectrums and spectrograms
        bool autoPowerAxis = default(true); // enables automatic power axis configuration
        double signalMinPower @unit(dBmW) = default(inf dBmW); // minimum signal power if axis is not automatic
        double signalMaxPower @unit(dBmW) = default(0 dBmW); // maximum signal power if axis is not automatic
        double signalMinPowerDensity @unit(dBmWpMHz) = default(inf dBmWpMHz); // minimum signal power density if axis is not automatic
        double signalMaxPowerDensity @unit(dBmWpMHz) = default(-inf dBmWpMHz); // maxium signal power density if axis is not automatic
        bool autoTimeAxis = default(true); // enables automatic time axis configuration
        double signalMinTime @unit(s) = default(0 s); // relative to current simulation time
        double signalMaxTime @unit(s) = default(0 s); // relative to current simulation time
        bool autoFrequencyAxis = default(true); // enables automatic frequency axis configuration
        double signalMinFrequency @unit(Hz) = default(inf Hz); // minimum signal frequency if axis is not automatic
        double signalMaxFrequency @unit(Hz) = default(-inf Hz); // maximum signal frequency if axis is not automatic

        // parameters for main power density map
        bool displayMainPowerDensityMap = default(false); // display a power density map for the transmission medium, can be enabled with dimensional analog models only
        double mainPowerDensityMapPixmapDensity = default(1); // pixmap relative resolution
        double mainPowerDensityMapMinX = default(0); // minimum X coordinate
        double mainPowerDensityMapMaxX = default(nan); // maximum X coordinate, compound module size by default
        double mainPowerDensityMapMinY = default(0); // minimum Y coordinate
        double mainPowerDensityMapMaxY = default(nan); // maximum Y coordinate, compound module size by default
        double mainPowerDensityMapZ = default(0); // Z coordinate
        int mainPowerDensityMapFigureXTickCount = default(3); // number of ticks along the X axis
        int mainPowerDensityMapFigureYTickCount = default(3); // number of ticks along the Y axis

        // parameters for power density maps
        bool displayPowerDensityMaps = default(false); // display power density maps at the nodes, can be enabled with dimensional analog models only
        string powerDensityMapMode @enum("total", "signal", "auto") = default("auto"); // total means total transmission medium, signal means transmitted or received signal, auto means total or signal and noise if transmission or reception is in progress
        string powerDensityMapPixelMode @enum("sample", "partition", "mean") = default("mean"); // specifices the method used to determine the color of each pixel, sample method is the least accurate but the fastest (sample < partition < mean)
        int powerDensityMapApproximationSize = default(10); // medium power density function approximation along X and Y axes
        double powerDensityMapCenterFrequency @mutable @unit(Hz) = default(nan Hz); // center frequency of medium power density sampling or integration
        double powerDensityMapBandwidth @mutable @unit(Hz) = default(0 Hz); // bandwidth of medium power density integration, 0 means sampling, inf means total integrated power
        double powerDensityMapFigureWidth = default(200); // plot figure width in pixels
        double powerDensityMapFigureHeight = default(120); // plot figure height in pixels
        double powerDensityMapPixmapWidth = default(powerDensityMapFigureWidth); // pixmap resolution width
        double powerDensityMapPixmapHeight = default(powerDensityMapFigureHeight); // pixmap resolution height
        double powerDensityMapZ = default(0); // Z coordinate
        int powerDensityMapFigureXTickCount = default(3); // number of ticks along the X axis
        int powerDensityMapFigureYTickCount = default(3); // number of ticks along the Y axis

        // parameters for spectrum figures
        bool displaySpectrums = default(false); // display power spectral density at the nodes, can be enabled with dimensional analog models only
        string spectrumMode @enum("total", "signal", "auto") = default("auto"); // total means total transmission medium, signal means transmitted or received signal, auto means total or signal and noise if transmission or reception is in progress
        double spectrumFigureWidth = default(200); // plot figure width in pixels
        double spectrumFigureHeight = default(120); // plot figure height in pixels
        int spectrumFigureXTickCount = default(3); // number of ticks along the X axis
        int spectrumFigureYTickCount = default(5); // number of ticks along the Y axis
        double spectrumFigureInterpolationSize = default(5); // power spectrum density interpolation distance along the frequency axis
        string spectrumPlacementHint = default("bottomCenter"); // annotation placement hint, space separated list of any, top, bottom, left, right, topLeft, topCenter, topRight, etc.
        double spectrumPlacementPriority = default(-1); // determines the order of annotation positioning

        // parameters for spectrogram figures
        bool displaySpectrograms = default(false); // display power density spectrograms at the nodes, can be enabled with dimensional analog models only
        string spectrogramMode @enum("total", "signal", "auto") = default("auto"); // total means total transmission medium, signal means transmitted or received signal, auto means total or signal and noise if transmission or reception is in progress
        string spectrogramPixelMode @enum("sample", "partition", "mean") = default("mean"); // specifices the method used to determine the color of each pixel, sample method is the least accurate but the fastest (sample < partition < mean)
        double spectrogramFigureWidth = default(200); // plot figure width in pixels
        double spectrogramFigureHeight = default(120); // plot figure height in pixels
        double spectrogramPixmapWidth = default(spectrogramFigureWidth); // pixmap resolution width
        double spectrogramPixmapHeight = default(spectrogramFigureHeight); // pixmap resolution height
        int spectrogramFigureXTickCount = default(3); // number of ticks along the X axis
        int spectrogramFigureYTickCount = default(5); // number of ticks along the Y axis
        string spectrogramPlacementHint = default("bottomCenter"); // annotation placement hint, space separated list of any, top, bottom, left, right, topLeft, topCenter, topRight, etc.
        double spectrogramPlacementPriority = default(-1); // determines the order of annotation positioning

        @class(MediumVisualizerBase);
}

