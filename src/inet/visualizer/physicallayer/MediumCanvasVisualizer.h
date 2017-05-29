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
#include "inet/physicallayer/contract/packetlevel/IRadioFrame.h"
#include "inet/physicallayer/contract/packetlevel/IReceptionDecision.h"
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
    std::vector<const ITransmission *> transmissions;
    /**
     * The list of transmission figures.
     */
    std::map<const IRadio *, cFigure *> transmissionFigures;
    /**
     * The list of reception figures.
     */
    std::map<const IRadio *, cFigure *> receptionFigures;
    /**
     * The propagating signal figures.
     */
    std::map<const ITransmission *, cFigure *> signalFigures;
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

    virtual cFigure *getTransmissionFigure(const IRadio *radio) const;
    virtual void setTransmissionFigure(const IRadio *radio, cFigure *figure);
    virtual cFigure *removeTransmissionFigure(const IRadio *radio);

    virtual cFigure *getReceptionFigure(const IRadio *radio) const;
    virtual void setReceptionFigure(const IRadio *radio, cFigure *figure);
    virtual cFigure *removeReceptionFigure(const IRadio *radio);

    virtual cFigure *getSignalFigure(const ITransmission *transmission) const;
    virtual void setSignalFigure(const ITransmission *transmission, cFigure *figure);
    virtual cFigure *removeSignalFigure(const ITransmission *transmission);

    virtual cGroupFigure *createSignalFigure(const ITransmission *transmission) const;
    virtual void refreshSignalFigure(const ITransmission *transmission) const;

    virtual void radioAdded(const IRadio *radio) override;
    virtual void radioRemoved(const IRadio *radio) override;

    virtual void transmissionAdded(const ITransmission *transmission) override;
    virtual void transmissionRemoved(const ITransmission *transmission) override;

    virtual void transmissionStarted(const ITransmission *transmission) override;
    virtual void transmissionEnded(const ITransmission *transmission) override;
    virtual void receptionStarted(const IReception *reception) override;
    virtual void receptionEnded(const IReception *reception) override;
};

} // namespace visualizer

} // namespace inet

#endif // ifndef __INET_MEDIUMCANVASVISUALIZER_H

