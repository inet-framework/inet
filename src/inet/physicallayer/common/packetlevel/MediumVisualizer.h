//
// Copyright (C) 2013 OpenSim Ltd.
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

#ifndef __INET_MEDIUMVISUALIZER_H
#define __INET_MEDIUMVISUALIZER_H

#include "inet/common/figures/HeatMapFigure.h"
#include "inet/common/figures/TrailFigure.h"
#include "inet/physicallayer/base/packetlevel/PhysicalLayerDefs.h"
#include "inet/physicallayer/contract/packetlevel/IRadioFrame.h"
#include "inet/physicallayer/contract/packetlevel/IReceptionResult.h"
#include "inet/physicallayer/contract/packetlevel/ITransmission.h"

namespace inet {

namespace physicallayer {

class RadioMedium;

/**
 * This class provides the visualization of the communication on the radio medium.
 */
class INET_API MediumVisualizer : public cSimpleModule
{
  protected:
    /** @name Parameters */
    //@{
    /**
     * The corresponding radio medium is never nullptr.
     */
    const RadioMedium *radioMedium;
    /**
     * Determines whether the visualizer displays the ongoing communications or not.
     */
    bool displayCommunication;
    /**
     * Determines how the ongoing communications are visualized: 3D spheres or
     * 2D circles on the X-Y plane.
     */
    bool drawCommunication2D;
    /**
     * Specifies whether successful communication between radios leave a trail or not.
     */
    bool leaveCommunicationTrail;

#if OMNETPP_VERSION >= 0x500
    /**
     * Specifies whether successful communication between radios leave a heat map or not.
     */
    bool leaveCommunicationHeat;
#endif

    /**
     * Specifies the size of the communication heat map.
     */
    int communicationHeatMapSize;
    //@}

    /** @name State */
    //@{
    /**
     * Update canvas interval when ongoing communication exists.
     */
    simtime_t updateCanvasInterval;
    /**
     * The list of ongoing transmissions.
     */
    std::vector<const ITransmission *> transmissions;
    //@}

    /** @name Timer */
    //@{
    /**
     * The message that is used to update the canvas when ongoing communications exist.
     */
    cMessage *updateCanvasTimer;
    //@}

    /** @name Figures */
    //@{
    /**
     * The layer figure that contains the figures representing the ongoing communications.
     */
    cGroupFigure *communicationLayer;
    /**
     * The trail figure that contains figures representing the recent successful communications.
     */
    TrailFigure *communicationTrail;

#if OMNETPP_VERSION >= 0x500
    /**
     * The heat map figure that shows the recent successful communications.
     */
    HeatMapFigure *communicationHeat;
#endif
    //@}

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;

    virtual void updateCanvas() const;
    virtual void scheduleUpdateCanvasTimer();

  public:
    MediumVisualizer();
    virtual ~MediumVisualizer();

    virtual void addTransmission(const ITransmission *transmission);
    virtual void removeTransmission(const ITransmission *transmission);

    virtual void receivePacket(const IReceptionResult *result);

    virtual void mediumChanged() const;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_MEDIUMVISUALIZER_H

