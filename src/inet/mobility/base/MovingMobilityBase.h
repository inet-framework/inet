/* -*- mode:c++ -*- ********************************************************
 * file:        MovingMobilityBase.h
 *
 * author:      Daniel Willkomm, Andras Varga
 *
 * copyright:   (C) 2004 Telecommunication Networks Group (TKN) at
 *              Technische Universitaet Berlin, Germany.
 *
 *              (C) 2005 Andras Varga
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 ***************************************************************************
 * part of:     framework implementation developed by tkn
 **************************************************************************/

#ifndef __INET_MOVINGMOBILITYBASE_H
#define __INET_MOVINGMOBILITYBASE_H

#include "inet/common/INETDefs.h"
#include "inet/mobility/base/MobilityBase.h"
#include "inet/common/figures/TrailFigure.h"

namespace inet {

/**
 * @brief Base class for moving mobility modules. Periodically emits a signal with the current mobility state.
 *
 * @ingroup mobility
 * @author Levente Meszaros
 */
class INET_API MovingMobilityBase : public MobilityBase
{
  protected:
    /** @brief The message used for mobility state changes. */
    cMessage *moveTimer;

    /** @brief The simulation time interval used to regularly signal mobility state changes.
     *
     * The 0 value turns off the signal. */
    simtime_t updateInterval;

    /** @brief A mobility model may decide to become stationary at any time.
     *
     * The true value disables sending self messages. */
    bool stationary;

    /** @brief The last speed that was reported at lastUpdate. */
    Coord lastSpeed;

    /** @brief The simulation time when the mobility state was last updated. */
    simtime_t lastUpdate;

    /** @brief The next simulation time when the mobility module needs to update its internal state.
     *
     * The -1 value turns off sending a self message for the next mobility state change. */
    simtime_t nextChange;

    /** @brief Draw the path on the canvas. */
    bool leaveMovementTrail;

    /** @brief The list of trail figures representing the movement. */
    TrailFigure *movementTrail;

  protected:
    MovingMobilityBase();

    virtual ~MovingMobilityBase();

    virtual void initialize(int stage) override;

    virtual void initializePosition() override;

    virtual void handleSelfMessage(cMessage *message) override;

    /** @brief Schedules the move timer that will update the mobility state. */
    void scheduleUpdate();

    /** @brief Moves and notifies listeners. */
    void moveAndUpdate();

    void updateVisualRepresentation() override;

    /** @brief Moves according to the mobility model to the current simulation time.
     *
     * Subclasses must override and update lastPosition, lastSpeed, lastUpdate, nextChange
     * and other state according to the mobility model.
     */
    virtual void move() = 0;

  public:
    /** @brief Returns the current position at the current simulation time. */
    virtual Coord getCurrentPosition() override;

    /** @brief Returns the current speed at the current simulation time. */
    virtual Coord getCurrentSpeed() override;
};

} // namespace inet

#endif // ifndef __INET_MOVINGMOBILITYBASE_H

