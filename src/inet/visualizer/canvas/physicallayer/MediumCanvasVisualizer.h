//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_MEDIUMCANVASVISUALIZER_H
#define __INET_MEDIUMCANVASVISUALIZER_H

#include "inet/common/ModuleRefByPar.h"
#include "inet/common/figures/HeatMapFigure.h"
#include "inet/common/figures/HeatMapPlotFigure.h"
#include "inet/common/figures/PlotFigure.h"
#include "inet/common/figures/TrailFigure.h"
#include "inet/common/geometry/common/CanvasProjection.h"
#include "inet/visualizer/base/MediumVisualizerBase.h"
#include "inet/visualizer/canvas/scene/NetworkNodeCanvasVisualizer.h"
#include "inet/visualizer/util/AnimationSpeedInterpolator.h"

#ifdef INET_WITH_PHYSICALLAYERWIRELESSCOMMON
#include "inet/physicallayer/wireless/common/contract/packetlevel/IReceptionDecision.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/ITransmission.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IWirelessSignal.h"
#endif // INET_WITH_PHYSICALLAYERWIRELESSCOMMON

namespace inet {

namespace visualizer {

class INET_API MediumCanvasVisualizer : public MediumVisualizerBase
{
#ifdef INET_WITH_PHYSICALLAYERWIRELESSCOMMON

  protected:
    /** @name Parameters */
    //@{
    double zIndex = NaN;
    const CanvasProjection *canvasProjection = nullptr;
    SignalShape signalShape = SIGNAL_SHAPE_RING;
    double signalOpacity = NaN;
    int signalRingCount = -1;
    double signalRingSize = NaN;
    double signalFadingDistance = NaN;
    double signalFadingFactor = NaN;
    int signalWaveCount = -1;
    double signalWaveLength = NaN;
    double signalWaveWidth = NaN;
    double signalWaveFadingAnimationSpeedFactor = NaN;
    bool displayCommunicationHeat = false;
    int communicationHeatMapSize = 100;
    //@}

    /** @name Internal state */
    //@{
    mutable bool invalidDisplay = true;
    mutable simtime_t lastRefreshDisplay;
    enum SignalInProgress {
        SIP_NONE,
        SIP_PROPAGATION,
        SIP_TRANSMISSION,
    };

    SignalInProgress lastSignalInProgress = SIP_NONE;
    AnimationSpeedInterpolator animationSpeedInterpolator;
    ModuleRefByPar<NetworkNodeCanvasVisualizer> networkNodeVisualizer;
    /**
     * The list of ongoing transmissions.
     */
    std::vector<const physicallayer::ITransmission *> transmissions;
    /**
     * The map of signal departure figures for all radios.
     */
    std::map<int, cFigure *> signalDepartureFigures;
    /**
     * The map of signal arrival figures for all radios.
     */
    std::map<int, cFigure *> signalArrivalFigures;
    /**
     * The propagating signal figures for all ongoing transmissions.
     */
    std::map<int, cFigure *> signalFigures;
    /**
     * The main power density figure.
     */
    HeatMapPlotFigure *mainPowerDensityMapFigure = nullptr;
    /**
     * The list of power density figures.
     */
    std::map<int, HeatMapPlotFigure *> powerDensityMapFigures;
    /**
     * The list of spectrum figures.
     */
    std::map<int, PlotFigure *> spectrumFigures;
    /**
     * The list of spectrum flow figures.
     */
    std::map<int, HeatMapPlotFigure *> spectrogramFigures;
    /**
     * The layer figure that contains the figures representing the ongoing communications.
     */
    cGroupFigure *signalLayer = nullptr;
    /**
     * The heat map figure that shows the recent successful communications.
     */
    HeatMapFigure *communicationHeat = nullptr;
    //@}

  protected:
    virtual void initialize(int stage) override;
    virtual void refreshDisplay() const override;
    virtual void setAnimationSpeed();

    virtual void refreshMainPowerDensityMapFigure() const;
    virtual void refreshPowerDensityMapFigure(const cModule *networkNode, HeatMapPlotFigure *figure) const;
    virtual void refreshPowerDensityMapFigurePowerFunction(const Ptr<const math::IFunction<WpHz, math::Domain<m, m, m, simsec, Hz>>>& powerDensityFunction, HeatMapPlotFigure *figure, int channel) const;

    virtual void refreshSpectrumFigure(const cModule *networkNode, PlotFigure *figure) const;
    virtual std::tuple<const physicallayer::ITransmission *, const physicallayer::ITransmission *, const physicallayer::IAntenna *, IMobility *> extractSpectrumFigureParameters(const cModule *networkNode) const;
    virtual void refreshSpectrumFigurePowerFunction(const Ptr<const math::IFunction<WpHz, math::Domain<m, m, m, simsec, Hz>>>& powerFunction, const physicallayer::IAntenna *antenna, const Coord& position, PlotFigure *figure, int series) const;
    virtual std::pair<WpHz, WpHz> computePowerForPartitionBounds(const Ptr<const math::IFunction<WpHz, math::Domain<m, m, m, simsec, Hz>>>& powerFunction, const math::Point<m, m, m, simsec, Hz>& lower, const math::Point<m, m, m, simsec, Hz>& upper, const math::IFunction<WpHz, math::Domain<m, m, m, simsec, Hz>> *partitonPowerFunction, const physicallayer::IAntenna *antenna, const Coord& position) const;
    virtual std::pair<WpHz, WpHz> computePowerForDirectionalAntenna(const Ptr<const math::IFunction<WpHz, math::Domain<m, m, m, simsec, Hz>>>& powerFunction, const math::Point<m, m, m, simsec, Hz>& lower, const math::Point<m, m, m, simsec, Hz>& upper, const physicallayer::IAntenna *antenna, const Coord& position) const;

    virtual void refreshSpectrogramFigure(const cModule *networkNode, HeatMapPlotFigure *figure) const;
    virtual void refreshSpectrogramFigurePowerFunction(const Ptr<const math::IFunction<WpHz, math::Domain<m, m, m, simsec, Hz>>>& powerFunction, const Coord& position, SimTimeUnit signalTimeUnit, HeatMapPlotFigure *figure, int channel) const;

    virtual cFigure *getSignalDepartureFigure(const physicallayer::IRadio *radio) const;
    virtual void setSignalDepartureFigure(const physicallayer::IRadio *radio, cFigure *figure);
    virtual cFigure *removeSignalDepartureFigure(const physicallayer::IRadio *radio);

    virtual cFigure *getSignalArrivalFigure(const physicallayer::IRadio *radio) const;
    virtual void setSignalArrivalFigure(const physicallayer::IRadio *radio, cFigure *figure);
    virtual cFigure *removeSignalArrivalFigure(const physicallayer::IRadio *radio);

    virtual cFigure *getSignalFigure(const physicallayer::ITransmission *transmission) const;
    virtual void setSignalFigure(const physicallayer::ITransmission *transmission, cFigure *figure);
    virtual cFigure *removeSignalFigure(const physicallayer::ITransmission *transmission);

    virtual cGroupFigure *createSignalFigure(const physicallayer::ITransmission *transmission) const;
    virtual void refreshSignalFigure(const physicallayer::ITransmission *transmission) const;

    virtual void handleRadioAdded(const physicallayer::IRadio *radio) override;
    virtual void handleRadioRemoved(const physicallayer::IRadio *radio) override;

    virtual void handleSignalAdded(const physicallayer::ITransmission *transmission) override;
    virtual void handleSignalRemoved(const physicallayer::ITransmission *transmission) override;

    virtual void handleSignalDepartureStarted(const physicallayer::ITransmission *transmission) override;
    virtual void handleSignalDepartureEnded(const physicallayer::ITransmission *transmission) override;
    virtual void handleSignalArrivalStarted(const physicallayer::IReception *reception) override;
    virtual void handleSignalArrivalEnded(const physicallayer::IReception *reception) override;

    virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details) override;
#endif // INET_WITH_PHYSICALLAYERWIRELESSCOMMON
};

} // namespace visualizer

} // namespace inet

#endif

