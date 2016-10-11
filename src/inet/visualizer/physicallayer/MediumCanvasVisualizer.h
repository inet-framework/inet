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
#include "inet/visualizer/networknode/NetworkNodeCanvasVisualizer.h"

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
    cImageFigure *transmissionImage = nullptr;
    cImageFigure *receptionImage = nullptr;
    bool displayCommunicationHeat = false;
    int communicationHeatMapSize = 100;
    //@}

    /** @name Internal state */
    //@{
    NetworkNodeCanvasVisualizer *networkNodeVisualizer = nullptr;
    /**
     * The list of ongoing transmissions.
     */
    std::vector<const ITransmission *> transmissions;
    /**
     * The list of ongoing transmission figures.
     */
    std::map<const ITransmission *, cFigure *> transmissionFigures;
    //@}

    /** @name Timer */
    //@{
    /**
     * The timer message that is used to update the canvas when propagating signals exist.
     */
    cMessage *signalPropagationUpdateTimer = nullptr;
    //@}

    /** @name Figures */
    //@{
    /**
     * The layer figure that contains the figures representing the ongoing communications.
     */
    cGroupFigure *communicationLayer = nullptr;
    /**
     * The layer figure that contains figures representing the recent radio frame sends.
     */
    cGroupFigure *radioFrameLayer = nullptr;
    /**
     * The heat map figure that shows the recent successful communications.
     */
    HeatMapFigure *communicationHeat = nullptr;
    //@}

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;
    virtual void refreshDisplay() const override;

    virtual cFigure *getCachedFigure(const ITransmission *transmission) const;
    virtual void setCachedFigure(const ITransmission *transmission, cFigure *figure);
    virtual void removeCachedFigure(const ITransmission *transmission);

    virtual void scheduleSignalPropagationUpdateTimer();

    virtual void radioAdded(const IRadio *radio) override;
    virtual void radioRemoved(const IRadio *radio) override;

    virtual void transmissionAdded(const ITransmission *transmission) override;
    virtual void transmissionRemoved(const ITransmission *transmission) override;

    virtual void transmissionStarted(const ITransmission *transmission) override;
    virtual void transmissionEnded(const ITransmission *transmission) override;
    virtual void receptionStarted(const IReception *reception) override;
    virtual void receptionEnded(const IReception *reception) override;

  public:
    virtual ~MediumCanvasVisualizer();
};

} // namespace visualizer

} // namespace inet

#endif // ifndef __INET_MEDIUMCANVASVISUALIZER_H

