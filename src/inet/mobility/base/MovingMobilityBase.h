//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//

#ifndef __INET_MOVINGMOBILITYBASE_H
#define __INET_MOVINGMOBILITYBASE_H

#include "inet/mobility/base/MobilityBase.h"

namespace inet {

/**
 * @brief Base class for moving mobility modules. Periodically emits a signal with the current mobility state.
 *
 * @ingroup mobility
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

    /** @brief The last velocity that was reported at lastUpdate. */
    Coord lastVelocity;

    /** @brief The last angular velocity that was reported at lastUpdate. */
    Quaternion lastAngularVelocity;

    /** @brief The simulation time when the mobility state was last updated. */
    simtime_t lastUpdate;

    /** @brief The next simulation time when the mobility module needs to update its internal state.
     *
     * The -1 value turns off sending a self message for the next mobility state change. */
    simtime_t nextChange;

    bool faceForward;

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

    /** @brief Moves according to the mobility model to the current simulation time.
     *
     * Subclasses must override and update lastPosition, lastVelocity, lastUpdate, nextChange
     * and other state according to the mobility model.
     */
    virtual void move() = 0;

    virtual void orient();

  public:
    virtual const Coord& getCurrentPosition() override;
    virtual const Coord& getCurrentVelocity() override;
    virtual const Coord& getCurrentAcceleration() override { throw cRuntimeError("Invalid operation"); }

    virtual const Quaternion& getCurrentAngularPosition() override;
    virtual const Quaternion& getCurrentAngularVelocity() override;
    virtual const Quaternion& getCurrentAngularAcceleration() override { throw cRuntimeError("Invalid operation"); }
};

} // namespace inet

#endif

