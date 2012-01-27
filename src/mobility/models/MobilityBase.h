/* -*- mode:c++ -*- ********************************************************
 * file:        MobilityBase.h
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


#ifndef MOBILITY_BASE_H
#define MOBILITY_BASE_H

#include "INETDefs.h"

#include "Coord.h"
#include "BasicModule.h"
#include "IMobility.h"


/**
 * @brief Abstract base class for mobility modules.
 *
 * Mobility modules inheriting from this base class may be stationary or may change
 * their mobility state over simulation time. This base class only provides a few
 * utility methods to handle some common tasks among mobility modules.
 *
 * Subclasses must redefine handleSelfMessage() to update the mobility state and
 * schedule the next update. Subclasses must also redefine initialize() to read their
 * specific parameters and schedule the first update.
 *
 * This base class also provides random initial placement as a default mechanism.
 * After initialization the module updates the display and emits a signal to subscribers
 * providing the mobility state. Receivers of this signal can query the mobility state
 * through the IMobility interface.
 *
 * @ingroup mobility
 * @ingroup basicModules
 * @author Daniel Willkomm, Andras Varga
 */
class INET_API MobilityBase : public BasicModule, public IMobility
{
  public:
    /**
     * Selects how a mobility module should behave if it reaches the edge of the constraint area.
     * @see handleIfOutside()
     */
    enum BorderPolicy {
        REFLECT,       ///< reflect off the wall
        WRAP,          ///< reappear at the opposite edge (torus)
        PLACERANDOMLY, ///< placed at a randomly chosen position within the constraint area
        RAISEERROR     ///< stop the simulation with error
    };

  protected:
    /** @brief A signal used to publish mobility state changes. */
    static simsignal_t mobilityStateChangedSignal;

    /** @brief Pointer to visual representation module, to speed up repeated access. */
    cModule* visualRepresentation;

    /** @brief 3 dimensional position and size of the constraint area (in meters). */
    Coord constraintAreaMin, constraintAreaMax;

    /** @brief The last position that was reported. */
    Coord lastPosition;

  protected:
    MobilityBase();

    /** @brief Returns the required number of initialize stages. */
    virtual int numInitStages() const {return 2;}

    /** @brief Initializes mobility model parameters in 4 stages. */
    virtual void initialize(int stage);

    /** @brief Initializes the position from the display string or from module parameters. */
    virtual void initializePosition();

    /** @brief This modules should only receive self-messages. */
    virtual void handleMessage(cMessage *msg);

    /** @brief Called upon arrival of a self messages, subclasses must override. */
    virtual void handleSelfMessage(cMessage *msg) = 0;

    /** @brief Moves the visual representation module's icon to the new position on the screen. */
    virtual void updateVisualRepresentation();

    /** @brief Emits a signal with the updated mobility state. */
    virtual void emitMobilityStateChangedSignal();

    /** @brief Returns a new random position satisfying the constraint area. */
    virtual Coord getRandomPosition();

    /** @brief Returns the module that represents the object moved by this mobility module. */
    virtual cModule *findVisualRepresentation() { return findHost(); }

    /** @brief Returns true if the mobility is outside of the constraint area. */
    virtual bool isOutside();

    /** @brief Utility function to reflect the node if it goes
     * outside the constraint area.
     *
     * Decision is made on pos, but the variables passed as args will
     * also be updated. (Pass dummies you don't have some of them).
     */
    virtual void reflectIfOutside(Coord& targetPosition, Coord& speed, double& angle);

    /** @brief Utility function to wrap the node to the opposite edge
     * (torus) if it goes outside the constraint area.
     *
     * Decision is made on pos, but targetPosition will also be updated.
     * (Pass a dummy you don't have it).
     */
    virtual void wrapIfOutside(Coord& targetPosition);

    /** @brief Utility function to place the node randomly if it goes
     * outside the constraint area.
     *
     * Decision is made on lastPosition, but targetPosition will also be updated.
     * (Pass a dummy you don't have it).
     */
    virtual void placeRandomlyIfOutside(Coord& targetPosition);

    /** @brief Utility function to raise an error if the node gets outside
     * the constraint area.
     */
    virtual void raiseErrorIfOutside();

    /** @brief Invokes one of reflectIfOutside(), wrapIfOutside() and
     * placeRandomlyIfOutside(), depending on the given border policy.
     */
    virtual void handleIfOutside(BorderPolicy policy, Coord& targetPosition, Coord& speed, double& angle);
};

#endif
