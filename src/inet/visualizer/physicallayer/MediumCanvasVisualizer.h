//
// Copyright (C) OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_MEDIUMCANVASVISUALIZER_H
#define __INET_MEDIUMCANVASVISUALIZER_H

#include "inet/common/figures/HeatMapFigure.h"
#include "inet/common/figures/TrailFigure.h"
#include "inet/common/geometry/common/CanvasProjection.h"
#include "inet/physicallayer/contract/packetlevel/IReceptionDecision.h"
#include "inet/physicallayer/contract/packetlevel/ISignal.h"
#include "inet/physicallayer/contract/packetlevel/ITransmission.h"
#include "inet/visualizer/base/MediumVisualizerBase.h"
#include "inet/visualizer/scene/NetworkNodeCanvasVisualizer.h"
#include "inet/visualizer/util/AnimationSpeedInterpolator.h"

namespace inet {

namespace visualizer {

class INET_API MediumCanvasVisualizer : public MediumVisualizerBase
{
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
    enum SignalInProgress {
        SIP_NONE,
        SIP_PROPAGATION,
        SIP_TRANSMISSION,
    };
    SignalInProgress lastSignalInProgress = SIP_NONE;
    AnimationSpeedInterpolator animationSpeedInterpolator;
    NetworkNodeCanvasVisualizer *networkNodeVisualizer = nullptr;
    /**
     * The list of ongoing transmissions.
     */
    std::vector<const physicallayer::ITransmission *> transmissions;
    /**
     * The list of transmission figures.
     */
    std::map<const physicallayer::IRadio *, cFigure *> signalDepartureFigures;
    /**
     * The list of reception figures.
     */
    std::map<const physicallayer::IRadio *, cFigure *> signalArrivalFigures;
    /**
     * The propagating signal figures.
     */
    std::map<const physicallayer::ITransmission *, cFigure *> signalFigures;
    //@}

    /** @name Figures */
    //@{
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
};

} // namespace visualizer

} // namespace inet

#endif // ifndef __INET_MEDIUMCANVASVISUALIZER_H

