/* -*- mode:c++ -*- ********************************************************
 * file:        Posture.h
 *
 * author:      Majid Nabi <m.nabi@tue.nl>
 *
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
 * description: A class to store the specification of a posture
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

#ifndef __INET_POSTURE_H
#define __INET_POSTURE_H

#include <string.h>

#include "inet/common/INETDefs.h"
#include "inet/common/INETMath.h"
#include "inet/common/geometry/common/Coord.h"

namespace inet {

/**
 * @brief to store the specification of a posture on the MoBAN mobility model.
 *
 *
 * @ingroup mobility
 * @ingroup MoBAN
 * @author Majid Nabi
 */
class INET_API Posture
{
  protected:
    /** @brief Number of nodes existing in the WBAN */
    unsigned int numNodes = 0;

    /** @brief The unique ID of the posture */
    unsigned int postureID = 0;

    /** @brief The relative reference position of each node in this posture */
    Coord *nodePs = nullptr;

    /** @brief A given name to the posture like walking, sitting.
     * It might be used for showing the current posture in the graphical interface during the simulation run */
    char *posture_name = nullptr;

    /** @brief Mean value of the normal distribution for the path lost coefficient (alpha)
     * for any pair of nodes (numNodes by numNodes matrix) */
    double **alphaMean = nullptr;

    /** @brief Mean value of the normal distribution for the path lost coefficient (alpha)
     * for any pair of nodes (numNodes by numNodes matrix) */
    double **alphaSD = nullptr;    // Standard deviation of alpha

    /** @brief Radious of the sphere around each node for individual mobility */
    double *nodeRadius = nullptr;

    /** @brief Movement speed of the node for individual mobility */
    double *nodeSpeed = nullptr;

    /** @brief Maximum value of the speed range for the global movement in this posture */
    double maxSpeed = NaN;

    /** @brief Minimum value of the speed range for the global movement in this posture */
    double minSpeed = NaN;

  public:
    /** @brief Construct a posture object with the given ID and number of nodes, respectively */
    Posture(unsigned int, unsigned int);

    ~Posture();

    /** @brief Return the number of nodes existing in the WBAN. */
    int getNumNodes() const { return numNodes; }

    /** @brief Set the given name for the posture. */
    bool setPostureName(char *);

    /** @brief Set the minimum and maximum value for the speed range of the whole WBAN (global movement) in this posture. */
    bool setPostureSpeed(double, double);

    /** @brief Set the radius of the sphere for movement of a singular node in this posture. */
    bool setRadius(unsigned int, double);

    /** @brief Set the velocity for movement of a singular node in this posture. */
    bool setSpeed(unsigned int, double);

    /** @brief Set the relative position of a node in this posture. */
    bool setPs(unsigned int, Coord);

    /** @brief Set the mean value of a normal distribution for path lost coefficient of wave propagation between two nodes in this posture. */
    bool setAlphaMean(unsigned int, unsigned int, double);

    /** @brief Set the standard deviation of a normal distribution for path lost coefficient of wave propagation between two nodes in this posture. */
    bool setAlphaSD(unsigned int, unsigned int, double);

    /** @brief Check if this posture is mobile by checking the maximum possible speed. */
    bool isMobile();

    /** @brief Returns the unique Id (index) of this posture. */
    int getPostureID();

    /** @brief Returns posture name. */
    char *getPostureName();

    /** @brief Returns maximum value of the speed range of this posture. */
    double getMaxSpeed();

    /** @brief Returns minimum value of the speed range of this posture. */
    double getMinSpeed();

    /** @brief Returns the singular movement radius of a node in this posture. */
    double getRadius(unsigned int);

    /** @brief Returns the singular movement speed of a node in this posture. */
    double getSpeed(unsigned int);

    /** @brief Returns the relative position of a node in this posture. */
    Coord getPs(unsigned int);

    /** @brief Returns the mean value of a normal distribution for path lost coefficient of wave propagation between two nodes in this posture. */
    double getAlphaMean(unsigned int, unsigned int);

    /** @brief Returns the standard deviation of a normal distribution for path lost coefficient of wave propagation between two nodes in this posture. */
    double getAlphaSD(unsigned int, unsigned int);
};

} // namespace inet

#endif // ifndef __INET_POSTURE_H

