/* -*- mode:c++ -*- ********************************************************
 * file:        BasicMobility.h
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


#ifndef BASIC_MOBILITY_H
#define BASIC_MOBILITY_H

#include <omnetpp.h>

#include "BasicModule.h"
#include "ChannelControl.h"
#include "Coord.h"


/**
 * @brief Abstract base class for all mobility modules.
 *
 * Subclasses are expected to redefine handleSelfMsg() to update the position
 * and schedule the time of the next position update, and initialize() to
 * read parameters and schedule the first position update.
 *
 * BasicMobility provides random placement of hosts and display
 * updates as well as registering with the ChannelControl module.
 * Change notifications about position changes are also posted to
 * NotificationBoard.
 *
 * @ingroup mobility
 * @ingroup basicModules
 * @author Daniel Willkomm, Andras Varga
 */
class INET_API BasicMobility : public BasicModule
{
  public:
    /**
     * Selects how a node should behave if it reaches the edge of the playground.
     * @sa handleIfOutside()
     */
    enum BorderPolicy {
        REFLECT,       ///< reflect off the wall
        WRAP,          ///< reappear at the opposite edge (torus)
        PLACERANDOMLY, ///< placed at a randomly chosen position on the blackboard
        RAISEERROR     ///< stop the simulation with error
    };

  protected:
    /** @brief Pointer to ChannelControl -- these two must know each other */
    ChannelControl *cc;

    /** @brief Identifies this host in the ChannelControl module*/
    ChannelControl::HostRef myHostRef;

    /** @brief Pointer to host module, to speed up repeated access*/
    cModule* hostPtr;

    /** @brief Stores the actual position of the host*/
    Coord pos;

  protected:
    /** @brief This modules should only receive self-messages*/
    virtual void handleMessage(cMessage *msg);

    /** @brief Initializes mobility model parameters.*/
    virtual void initialize(int);

    /** @brief Delete dynamically allocated objects*/
    virtual void finish() {}

  protected:
    /** @brief Called upon arrival of a self messages*/
    virtual void handleSelfMsg(cMessage *msg) = 0;

    /** @brief Update the position information for this node.
     *
     * This function tells NotificationBoard that the position has changed, and
     * it also moves the host's icon to the new position on the screen.
     *
     * This function has to be called every time the position of the host
     * changes!
     */
    virtual void updatePosition();

    /** @brief Returns the width of the playground */
    virtual double getPlaygroundSizeX() const  {return cc->getPgs()->x;}

    /** @brief Returns the height of the playground */
    virtual double getPlaygroundSizeY() const  {return cc->getPgs()->y;}

    /** @brief Get a new random position for the host*/
    virtual Coord getRandomPosition();

    /** @brief Utility function to reflect the node if it goes
     * outside the playground.
     *
     * Decision is made on pos, but the variables passed as args will
     * also be updated. (Pass dummies you don't have some of them).
     */
    virtual void reflectIfOutside(Coord& targetPos, Coord& step, double& angle);

    /** @brief Utility function to wrap the node to the opposite edge
     * (torus) if it goes outside the playground.
     *
     * Decision is made on pos, but targetPos will also be updated.
     * (Pass a dummy you don't have it).
     */
    virtual void wrapIfOutside(Coord& targetPos);

    /** @brief Utility function to place the node randomly if it goes
     * outside the playground.
     *
     * Decision is made on pos, but targetPos will also be updated.
     * (Pass a dummy you don't have it).
     */
    virtual void placeRandomlyIfOutside(Coord& targetPos);

    /** @brief Utility function to raise an error if the node gets outside
     * the playground.
     */
    virtual void raiseErrorIfOutside();

    /** @brief Invokes one of reflectIfOutside(), wrapIfOutside() and
     * placeRandomlyIfOutside(), depending on the given border policy.
     */
    virtual void handleIfOutside(BorderPolicy policy, Coord& targetPos, Coord& step, double& angle);

};

#endif

