//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_MEDIUMVISUALIZERBASE_H
#define __INET_MEDIUMVISUALIZERBASE_H

#include "inet/common/packet/PacketFilter.h"
#include "inet/visualizer/base/VisualizerBase.h"
#include "inet/visualizer/util/AnimationSpeedInterpolator.h"
#include "inet/visualizer/util/ColorSet.h"
#include "inet/visualizer/util/InterfaceFilter.h"
#include "inet/visualizer/util/NetworkNodeFilter.h"
#include "inet/visualizer/util/Placement.h"

#ifdef INET_WITH_PHYSICALLAYERWIRELESSCOMMON
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadioMedium.h"
#include "inet/physicallayer/wireless/common/signal/PowerFunctions.h"
#endif // INET_WITH_PHYSICALLAYERWIRELESSCOMMON

namespace inet {

namespace visualizer {


// TODO: visualization of temporal and spectral and spatial energy density using sampling/filling/integrating method
class INET_API MediumVisualizerBase : public VisualizerBase, public cListener
{
#ifdef INET_WITH_PHYSICALLAYERWIRELESSCOMMON
  protected:
    enum SignalShape {
        SIGNAL_SHAPE_RING,
        SIGNAL_SHAPE_SPHERE,
        SIGNAL_SHAPE_BOTH,
    };

  protected:
    /** @name Parameters */
    //@{
    physicallayer::IRadioMedium *radioMedium = nullptr;
    NetworkNodeFilter networkNodeFilter;
    InterfaceFilter interfaceFilter;
    PacketFilter packetFilter;
    bool displaySignals = false;
    ColorSet signalColorSet;
    double signalPropagationAnimationSpeed = NaN;
    double signalPropagationAnimationTime = NaN;
    double signalPropagationAdditionalTime = NaN;
    double signalTransmissionAnimationSpeed = NaN;
    double signalTransmissionAnimationTime = NaN;
    AnimationPosition::TimeType signalAnimationSpeedChangeTimeMode = AnimationPosition::REAL_TIME;
    double signalAnimationSpeedChangeTime = NaN;
    bool displaySignalDepartures = false;
    bool displaySignalArrivals = false;
    Placement signalDeparturePlacementHint;
    Placement signalArrivalPlacementHint;
    double signalDeparturePlacementPriority;
    double signalArrivalPlacementPriority;
    bool displayInterferenceRanges = false;
    cFigure::Color interferenceRangeLineColor;
    cFigure::LineStyle interferenceRangeLineStyle;
    double interferenceRangeLineWidth = NaN;
    bool displayCommunicationRanges = false;
    cFigure::Color communicationRangeLineColor;
    cFigure::LineStyle communicationRangeLineStyle;
    double communicationRangeLineWidth = NaN;
    bool autoPowerAxis = false;
    W signalMinPower = W(NaN);
    W signalMaxPower = W(NaN);
    WpHz signalMinPowerDensity = WpHz(NaN);
    WpHz signalMaxPowerDensity = WpHz(NaN);
    bool autoTimeAxis = false;
    simtime_t signalMinTime;
    simtime_t signalMaxTime;
    bool autoFrequencyAxis = false;
    Hz signalMinFrequency = Hz(NaN);
    Hz signalMaxFrequency = Hz(NaN);
    bool displayMainPowerDensityMap = false;
    double mainPowerDensityMapPixmapDensity = NaN;
    double mainPowerDensityMapMinX = NaN;
    double mainPowerDensityMapMaxX = NaN;
    double mainPowerDensityMapMinY = NaN;
    double mainPowerDensityMapMaxY = NaN;
    double mainPowerDensityMapZ = NaN;
    int mainPowerDensityMapFigureXTickCount = -1;
    int mainPowerDensityMapFigureYTickCount = -1;
    bool displayPowerDensityMaps = false;
    const char *powerDensityMapMode = nullptr;
    const char *powerDensityMapPixelMode = nullptr;
    int powerDensityMapApproximationSize = -1;
    Hz powerDensityMapCenterFrequency = Hz(NaN);
    Hz powerDensityMapBandwidth = Hz(NaN);
    double powerDensityMapFigureWidth = NaN;
    double powerDensityMapFigureHeight = NaN;
    double powerDensityMapPixmapWidth = NaN;
    double powerDensityMapPixmapHeight = NaN;
    double powerDensityMapZ = NaN;
    int powerDensityMapFigureXTickCount = -1;
    int powerDensityMapFigureYTickCount = -1;
    bool displaySpectrums = false;
    const char *spectrumMode = nullptr;
    double spectrumFigureWidth = NaN;
    double spectrumFigureHeight = NaN;
    int spectrumFigureXTickCount = -1;
    int spectrumFigureYTickCount = -1;
    double spectrumFigureInterpolationSize = NaN;
    Placement spectrumPlacementHint;
    double spectrumPlacementPriority;
    bool displaySpectrograms = false;
    const char *spectrogramMode = nullptr;
    const char *spectrogramPixelMode = nullptr;
    double spectrogramFigureWidth = NaN;
    double spectrogramFigureHeight = NaN;
    double spectrogramPixmapWidth = NaN;
    double spectrogramPixmapHeight = NaN;
    int spectrogramFigureXTickCount = -1;
    int spectrogramFigureYTickCount = -1;
    Placement spectrogramPlacementHint;
    double spectrogramPlacementPriority;
    //@}

    /** @name State */
    //@{
    Ptr<const math::IFunction<double, math::Domain<mps, m, Hz>>> pathLossFunction;
    Ptr<const math::IFunction<double, math::Domain<m, m, m, m, m, m, Hz>>> obstacleLossFunction;
    double defaultSignalPropagationAnimationSpeed = NaN;
    double defaultSignalTransmissionAnimationSpeed = NaN;
    std::map<int, Ptr<const math::IFunction<WpHz, math::Domain<m, m, m, simsec, Hz>>>> signalPowerDensityFunctions;
    std::map<int, Ptr<math::SummedFunction<WpHz, math::Domain<m, m, m, simsec, Hz>>>> noisePowerDensityFunctions;
    Ptr<math::SummedFunction<WpHz, math::Domain<m, m, m, simsec, Hz>>> mediumPowerDensityFunction;
    //@}

  protected:
    virtual void initialize(int stage) override;
    virtual void handleParameterChange(const char *name) override;
    virtual void preDelete(cComponent *root) override;

    virtual bool isSignalPropagationInProgress(const physicallayer::ITransmission *transmission) const;
    virtual bool isSignalTransmissionInProgress(const physicallayer::ITransmission *transmission) const;

    virtual bool matchesTransmission(const physicallayer::ITransmission *transmission) const;

    virtual void handleRadioAdded(const physicallayer::IRadio *radio) = 0;
    virtual void handleRadioRemoved(const physicallayer::IRadio *radio) = 0;

    virtual void handleSignalAdded(const physicallayer::ITransmission *transmission);
    virtual void handleSignalRemoved(const physicallayer::ITransmission *transmission);

    virtual void handleSignalDepartureStarted(const physicallayer::ITransmission *transmission) = 0;
    virtual void handleSignalDepartureEnded(const physicallayer::ITransmission *transmission) = 0;
    virtual void handleSignalArrivalStarted(const physicallayer::IReception *reception);
    virtual void handleSignalArrivalEnded(const physicallayer::IReception *reception) = 0;

  public:
    virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details) override;
#endif // INET_WITH_PHYSICALLAYERWIRELESSCOMMON
};

} // namespace visualizer

} // namespace inet

#endif

