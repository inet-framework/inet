/* -*- mode:c++ -*- ********************************************************
 * file:        MoBANLocal.h
 *
 * author:      Majid Nabi <m.nabi@tue.nl>
 *
 *              http://www.es.ele.tue.nl/nes
 *
 * copyright:   (C) 2010 Electronic Systems group(ES),
 *              Eindhoven University of Technology (TU/e), the Netherlands.
 *
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 ***************************************************************************
 * part of:    MoBAN (Mobility Model for wireless Body Area Networks)
 * description:     Implementation of the local module of the MoBAN mobility model
 ***************************************************************************
 * Citation of the following publication is appreciated if you use MoBAN for
 * a publication of your own.
 *
 * M. Nabi, M. Geilen, T. Basten. MoBAN: A Configurable Mobility Model for Wireless Body Area Networks.
 * In Proc. of the 4th Int'l Conf. on Simulation Tools and Techniques, SIMUTools 2011, Barcelona, Spain, 2011.
 *
 * BibTeX:
 *        @inproceedings{MoBAN,
 *         author = "M. Nabi and M. Geilen and T. Basten.",
 *          title = "{MoBAN}: A Configurable Mobility Model for Wireless Body Area Networks.",
 *        booktitle = "Proceedings of the 4th Int'l Conf. on Simulation Tools and Techniques.",
 *        series = {SIMUTools '11},
 *        isbn = {978-963-9799-41-7},
 *        year = {2011},
 *        location = {Barcelona, Spain},
 *        publisher = {ICST} }
 *
 **************************************************************************/

#ifndef __INET_MOBANLOCAL_H
#define __INET_MOBANLOCAL_H

#include "inet/common/INETDefs.h"

#include "inet/mobility/base/LineSegmentsMobilityBase.h"
#include "inet/mobility/group/MoBANCoordinator.h"

namespace inet {

/**
 * @brief This is the local mobility module of MoBAN. It should be instantiated in each node that belongs to a WBAN.
 * The NED parameter "coordinatorIndex" determines to which WBAN (MoBANCoordinator) it belongs.
 * The current implementation uses the Random Walk Mobility Model (RWMM) for individual (local) movement within a sphere around the node, with given speed
 * and sphere radius of the current posture. The reference point of the node in the current posture, the sphere radius, and the speed is given by the
 * corresponding coordinator. RWMM determines the location of the node at any time relative to the given reference point.
 *
 * @ingroup mobility
 * @ingroup MoBAN
 * @author Majid Nabi
 */
class INET_API MoBANLocal : public LineSegmentsMobilityBase
{
  protected:
    /** @brief The coordinator of the WBAN. */
    MoBANCoordinator *coordinator;

    /** @brief Reference position of the node in the current posture. */
    Coord referencePosition;

    /** @brief The radius of local mobility of the node in the current posture. */
    double radius;

    /** @brief The speed of local mobility of the node in the current posture. */
    double speed;

    /** @brief The possible maximum speed at any future time */
    double maxSpeed;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }

    virtual void initialize(int) override;

    virtual void setInitialPosition() override;

    virtual void setTargetPosition() override;

    virtual void updateVisualRepresentation() override;

    virtual void computeMaxSpeed();
  public:
    MoBANLocal();

    virtual Coord getCurrentPosition() override;

    virtual Coord getCurrentSpeed() override;

    void setCoordinator(MoBANCoordinator *coordinator) { this->coordinator = coordinator; }

    void setMoBANParameters(Coord referencePoint, double radius, double speed);

    virtual double getMaxSpeed() const override { return maxSpeed; }
};

} // namespace inet

#endif // ifndef __INET_MOBANLOCAL_H

