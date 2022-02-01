//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ROTATINGMOBILITYBASE_H
#define __INET_ROTATINGMOBILITYBASE_H

#include "inet/mobility/base/MovingMobilityBase.h"

namespace inet {

class INET_API RotatingMobilityBase : public MobilityBase
{
  protected:
    /** @brief The message used for mobility state changes. */
    cMessage *rotateTimer;

    /** @brief The simulation time interval used to regularly signal mobility state changes.
     *
     * The 0 value turns off the signal. */
    simtime_t updateInterval;

    /** @brief A mobility model may decide to become stationary at any time.
     *
     * The true value disables sending self messages. */
    bool stationary;

    /** @brief The last velocity that was reported at lastUpdate. */
    Quaternion lastAngularVelocity;

    /** @brief The simulation time when the mobility state was last updated. */
    simtime_t lastUpdate;

    /** @brief The next simulation time when the mobility module needs to update its internal state.
     *
     * The -1 value turns off sending a self message for the next mobility state change. */
    simtime_t nextChange;

  protected:
    RotatingMobilityBase();

    virtual ~RotatingMobilityBase();

    virtual void initialize(int stage) override;

    virtual void initializeOrientation() override;

    virtual void handleSelfMessage(cMessage *message) override;

    /** @brief Schedules the move timer that will update the mobility state. */
    void scheduleUpdate();

    /** @brief Moves and notifies listeners. */
    void rotateAndUpdate();

    /** @brief Moves according to the mobility model to the current simulation time.
     *
     * Subclasses must override and update lastPosition, lastVelocity, lastUpdate, nextChange
     * and other state according to the mobility model.
     */
    virtual void rotate() = 0;

  public:
    /** @brief Returns the current angular position at the current simulation time. */
    virtual const Quaternion& getCurrentAngularPosition() override;

    /** @brief Returns the current angular velocity at the current simulation time. */
    virtual const Quaternion& getCurrentAngularVelocity() override;
};

} // namespace inet

#endif

