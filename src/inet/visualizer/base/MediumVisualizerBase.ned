//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.base;

//
// This is a base module for medium visualizer simple modules. It provides
// visualization of wireless signal propagation and radio medium
// properties in the network. TODO radio medium properties?
//
// The visualizer offers multiple visualization options:
// - Propagating signals as expanding circles
// - Signal departures and arrivals with icons
// - Communication and interference ranges as circles around radios
// - Power density maps showing signal strength distribution
// - Spectrum figures displaying power spectral density
// - Spectrograms showing power density over time and frequency
//
// By default, all visualizations are disabled. Each can be enabled individually
// through the corresponding display parameters.
//
// This module subscribes to `signalSourceAdded`, `signalSourceRemoved`, `signalAdded`,
// `signalRemoved`, `signalDepartureStarted`, `signalDepartureEnded`, `signalArrivalStarted`,
// `signalArrivalEnded` simulation signals at the module determined by the `mediumModule`
// parameter. The actual rendering of the visualizations is implemented in derived
// modules for specific user interfaces (canvas or OSG).
//
// The medium visualizer is particularly useful for understanding wireless signal
// propagation, analyzing interference patterns, debugging radio communication
// issues, and visualizing spectral properties of the wireless medium.
//
// @see ~MediumCanvasVisualizer, ~MediumOsgVisualizer, ~IMediumVisualizer, ~VisualizerBase
//
simple MediumVisualizerBase extends VisualizerBase
{
    parameters:
        string mediumModule = default("radioMedium"); // The medium module that is displayed, refers to a top-level submodule by default
        string networkNodeVisualizerModule = default("^.networkNodeVisualizer"); // The network node visualizer module, refers to a sibling submodule by default

        string nodeFilter @mutable = default("*"); // Which nodes are considered, matches all nodes by default
        string interfaceFilter @mutable = default("*"); // Which interfaces are considered, matches all interfaces by default
        object packetFilter @mutable = default("*"); // Which packets are considered, matches all packets by default

        bool displaySignals = default(false); // Display signals propagating through the medium, disabled by default
        string signalColor = default("dark"); // Signal color is a list of colors or a color group name (e.g., dark, light), a set of dark colors by default
        double signalPropagationAnimationSpeed @mutable = default(nan); // Animation speed while the very beginning or end of a signal is propagating on the medium, value must be in the range (0, +inf) or NaN, the latter means automatic setting
        double signalPropagationAnimationTime @unit(s) = default(1s); // Signal propagation animation duration when signalPropagationAnimationSpeed is unspecified
        double signalPropagationAdditionalTime @unit(s) = default(0s); // Additional simulation time to animate signal propagation after leaving the last receiver
        double signalTransmissionAnimationSpeed @mutable = default(nan); // Animation speed while a signal is being transmitted on the medium, value must be in the range (0, +inf) or NaN, the latter means automatic setting
        double signalTransmissionAnimationTime @unit(s) = default(1s); // Signal transmission animation duration when signalTransmissionAnimationSpeed is unspecified
        string signalAnimationSpeedChangeTimeMode @enum("realTime", "animationTime", "simulationTime") = default("realTime"); // Specifies the time mode for animation speed changes between propagation and transmission animation
        double signalAnimationSpeedChangeTime @unit(s) = default(1s); // Animation speed changes smoothly from propagationAnimationSpeed to transmissionAnimationSpeed in the specified real time

        bool displaySignalDepartures = default(false); // Display an image where a signal departure is currently in progress, disabled by default
        string signalDepartureImage = default("misc/signal_departure.png"); // Image for ongoing signal departures
        string signalDeparturePlacementHint = default("topCenter"); // Annotation placement hint, space-separated list of any, top, bottom, left, right, topLeft, topCenter, topRight, etc.
        double signalDeparturePlacementPriority = default(1); // Determines the order of annotation positioning

        bool displaySignalArrivals = default(false); // Display an image where a signal arrival is currently in progress, disabled by default
        string signalArrivalImage = default("misc/signal_arrival.png"); // Image for ongoing signal arrivals
        string signalArrivalPlacementHint = default("topCenter"); // Annotation placement hint, space-separated list of any, top, bottom, left, right, topLeft, topCenter, topRight, etc.
        double signalArrivalPlacementPriority = default(-1); // Determines the order of annotation positioning

        bool displayCommunicationRanges = default(false); // Display communication ranges as circles around radios, enabled by default
        string communicationRangeLineColor = default("blue"); // Communication range circle line color, blue by default
        string communicationRangeLineStyle = default("solid"); // Communication range circle line style, solid by default
        double communicationRangeLineWidth = default(1); // Communication range circle line width, 1 by default

        bool displayInterferenceRanges = default(false); // Display interference ranges as circles around radios, disabled by default
        string interferenceRangeLineColor = default("gray"); // Interference range circle color, gray by default
        string interferenceRangeLineStyle = default("solid"); // Interference range circle line style, solid by default
        double interferenceRangeLineWidth = default(1); // Interference range circle line width, 1 by default

        // parameters shared between power density maps, spectrums, and spectrograms
        bool autoPowerAxis = default(true); // Enables automatic power axis configuration
        double signalMinPower @unit(dBmW) = default(inf dBmW); // Minimum signal power if the axis is not automatic
        double signalMaxPower @unit(dBmW) = default(0 dBmW); // Maximum signal power if the axis is not automatic
        double signalMinPowerDensity @unit(dBmWpMHz) = default(inf dBmWpMHz); // Minimum signal power density if the axis is not automatic
        double signalMaxPowerDensity @unit(dBmWpMHz) = default(-inf dBmWpMHz); // Maximum signal power density if the axis is not automatic
        bool autoTimeAxis = default(true); // Enables automatic time axis configuration
        double signalMinTime @unit(s) = default(0 s); // Relative to the current simulation time
        double signalMaxTime @unit(s) = default(0 s); // Relative to the current simulation time
        bool autoFrequencyAxis = default(true); // Enables automatic frequency axis configuration
        double signalMinFrequency @unit(Hz) = default(inf Hz); // Minimum signal frequency if the axis is not automatic
        double signalMaxFrequency @unit(Hz) = default(-inf Hz); // Maximum signal frequency if the axis is not automatic

        // parameters for the main power density map
        bool displayMainPowerDensityMap = default(false); // Display a power density map for the transmission medium, can be enabled with dimensional analog models only
        double mainPowerDensityMapPixmapDensity = default(1); // Pixmap relative resolution
        double mainPowerDensityMapMinX = default(0); // Minimum X coordinate
        double mainPowerDensityMapMaxX = default(nan); // Maximum X coordinate, compound module size by default
        double mainPowerDensityMapMinY = default(0); // Minimum Y coordinate
        double mainPowerDensityMapMaxY = default(nan); // Maximum Y coordinate, compound module size by default
        double mainPowerDensityMapZ = default(0); // Z coordinate
        int mainPowerDensityMapFigureXTickCount = default(3); // Number of ticks along the X-axis
        int mainPowerDensityMapFigureYTickCount = default(3); // Number of ticks along the Y-axis

        // parameters for power density maps
        bool displayPowerDensityMaps = default(false); // Display power density maps at the nodes, can be enabled with dimensional analog models only
        string powerDensityMapMode @enum("total", "signal", "auto") = default("auto"); // Total means total transmission medium, signal means transmitted or received signal, auto means total or signal and noise if transmission or reception is in progress
        string powerDensityMapPixelMode @enum("sample", "partition", "mean") = default("mean"); // Specifies the method used to determine the color of each pixel, the sample method is the least accurate but the fastest (sample < partition < mean)
        int powerDensityMapApproximationSize = default(10); // Medium power density function approximation along the X and Y axes
        double powerDensityMapCenterFrequency @mutable @unit(Hz) = default(nan Hz); // Center frequency of medium power density sampling or integration
        double powerDensityMapBandwidth @mutable @unit(Hz) = default(0 Hz); // Bandwidth of medium power density integration, 0 means sampling, inf means total integrated power
        double powerDensityMapFigureWidth = default(200); // Plot figure width in pixels
        double powerDensityMapFigureHeight = default(120); // Plot figure height in pixels
        double powerDensityMapPixmapWidth = default(powerDensityMapFigureWidth); // Pixmap resolution width
        double powerDensityMapPixmapHeight = default(powerDensityMapFigureHeight); // Pixmap resolution height
        double powerDensityMapZ = default(0); // Z coordinate
        int powerDensityMapFigureXTickCount = default(3); // Number of ticks along the X-axis
        int powerDensityMapFigureYTickCount = default(3); // Number of ticks along the Y-axis

        // parameters for spectrum figures
        bool displaySpectrums = default(false); // Display power spectral density at the nodes, can be enabled with dimensional analog models only
        string spectrumMode @enum("total", "signal", "auto") = default("auto"); // Total means total transmission medium, signal means transmitted or received signal, auto means total or signal and noise if transmission or reception is in progress
        double spectrumFigureWidth = default(200); // Plot figure width in pixels
        double spectrumFigureHeight = default(120); // Plot figure height in pixels
        int spectrumFigureXTickCount = default(3); // Number of ticks along the X-axis
        int spectrumFigureYTickCount = default(5); // Number of ticks along the Y-axis
        double spectrumFigureInterpolationSize = default(5); // Power spectrum density interpolation distance along the frequency axis
        string spectrumPlacementHint = default("bottomCenter"); // Annotation placement hint, space-separated list of any, top, bottom, left, right, topLeft, topCenter, topRight, etc.
        double spectrumPlacementPriority = default(-1); // Determines the order of annotation positioning

        // parameters for the spectrogram figures
        bool displaySpectrograms = default(false); // Display power density spectrograms at the nodes, can be enabled with dimensional analog models only
        string spectrogramMode @enum("total", "signal", "auto") = default("auto"); // Total means total transmission medium, signal means transmitted or received signal, auto means total or signal and noise if transmission or reception is in progress
        string spectrogramPixelMode @enum("sample", "partition", "mean") = default("mean"); // Specifies the method used to determine the color of each pixel, sample method is the least accurate but the fastest (sample < partition < mean)
        double spectrogramFigureWidth = default(200); // Plot figure width in pixels
        double spectrogramFigureHeight = default(120); // Plot figure height in pixels
        double spectrogramPixmapWidth = default(spectrogramFigureWidth); // Pixmap resolution width
        double spectrogramPixmapHeight = default(spectrogramFigureHeight); // Pixmap resolution height
        int spectrogramFigureXTickCount = default(3); // Number of ticks along the X-axis
        int spectrogramFigureYTickCount = default(5); // Number of ticks along the Y-axis
        string spectrogramPlacementHint = default("bottomCenter"); // Annotation placement hint, space-separated list of any, top, bottom, left, right, topLeft, topCenter, topRight, etc.
        double spectrogramPlacementPriority = default(-1); // Determines the order of annotation positioning

        @class(MediumVisualizerBase);
}
