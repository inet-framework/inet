/* -*- mode:c++ -*- ********************************************************
 * file:        PostureTransition.h
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
 * description: A class to manage and store the posture transition matrices
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

#ifndef __INET_POSTURETRANSITION_H
#define __INET_POSTURETRANSITION_H

#include <iostream>
#include <sstream>

#include "inet/common/INETDefs.h"
#include "inet/common/geometry/common/Coord.h"

namespace inet {

/**
 * @brief Class to provide spatial and temporal correlation in the posture selection process of the MoBAN mobility model.
 * This class obtains and stores Markovian transition matrices. There is also the possibility to get a steady state vector. In this
 * case, the closest transition matrix to the default Makov matrix is extracted which satisfies the given steady state vector.
 * The class also receives the defined area types and time domains as well as given space-time domains during the initialization phase.
 * During the simulation run, the class provide a functions to return the corresponding markov matrix for a given time and location. It will
 * be used whenever a new posture is going to be selected.
 *
 *
 * @ingroup mobility
 * @ingroup MoBAN
 * @author Majid Nabi
 */
class INET_API PostureTransition
{
  protected:
    /** @brief Number of postures. */
    int numPos;

    /** @brief The index of the default (base) transition matrix. If no default is set, the first matrix is supposed as the default.
     * Default matrix is used for the cases that a time or space domain does not lie in any given area types or time domains.
     * It is also used for generating the transition matrix in the case that a steady state vector is given for a space-time domain.
     */
    int defaultMatrixID;

    /** @brief Data type for one instance of Markov transition matrix. */
    typedef struct
    {
        std::string name;
        double **matrix;
    } TransMatrix;

    /** @brief Data type for a list of Markov transition matrices. */
    typedef std::vector<TransMatrix *> TransMatrixList;

    /** @brief The list of all given transition matrices. */
    TransMatrixList matrixList;

    /** @brief Data type for one instance of the area (space) boundary. */
    typedef struct
    {
        Coord low;
        Coord high;
    } AreaBound;

    /** @brief Data type for one instance of area type. */
    typedef struct
    {
        std::string name;
        std::vector<AreaBound *> boundries;
    } AreaType;

    /** @brief Data type for the list of area types. */
    typedef std::vector<AreaType *> AreaTypeList;

    /** @brief The list of all defined area types. */
    AreaTypeList areaTypeList;

    /** @brief Data type for one instance of the time boundary. */
    typedef struct
    {
        simtime_t low;
        simtime_t high;
    } TimeBound;

    /** @brief Data type for one instance of time domain. */
    typedef struct
    {
        std::string name;
        std::vector<TimeBound *> boundries;
    } TimeDomainType;

    /** @brief Data type for the list of time domains. */
    typedef std::vector<TimeDomainType *> TimeDomainList;

    /** @brief The list of all defined time domains. */
    TimeDomainList timeDomainList;

    /** @brief Data type for one instance of space-time combination. */
    typedef struct
    {
        int timeID;
        int areaID;
        int matrixID;
    } CombinationType;

    /** @brief Data type for the list of space-time combinations. */
    typedef std::vector<CombinationType *> CombinationList;

    /** @brief The list of all given space-time combinations. */
    CombinationList combinationList;

    /** @brief Gets a steady state vector and return a matrix which is as close as posible to the default matrix
     * and satisfies the given steady state.
     */
    double **extractMatrixFromSteadyState(double *);

    /** @brief Gets a time and finds the ID of the containing time domain if there is. If not, return -1. */
    int findTimeDomain(simtime_t);

    /** @brief Gets a location and finds the ID of the containing area type if there is. If not, return -1. */
    int findAreaType(Coord);

    /** @brief Checks if a matrix can be a Markov transition matrix. All elements should be in the range [0,1]
     * and elements of each column of the matrix should add up to 1.
     */
    bool isMarkovian(double **);

    /** @brief Checks if a vector can be the steady state of a Markov chain. All elements should be in the range [0,1]
     * and the sum of elements should be 1.
     */
    bool isMarkovian(double *);

    /** @brief Multiplies two matrices with dimension numPos*numPose . */
    void multMatrix(double **, double **, double **);

    /** @brief Adds two matrices with dimension numPos*numPose . */
    void addMatrix(double **, double **, double **);

    /** @brief Subtracts two matrices with dimension numPos*numPose . */
    void subtractMatrix(double **, double **, double **);

    /** @brief Multiply a vector of size numPos with its transpose. */
    void multVector(double *, double **);

  public:
    /** @brief Construct a posture transition object. The parameter is the number of postures which is
     * the dimension of all matrices
     */
    PostureTransition(int);

    ~PostureTransition();

    /** @brief Receives a transition matrix and add to the list. */
    int addMatrix(std::string, double **, bool);

    /** @brief Receives a steady state vector, extracts the corresponding transition matrix
     * considering the default matrix, and add to the list of given matrices
     */
    int addSteadyState(std::string, double *);

    /** @brief Adds a area type to the list with the given name and returns the index of this area type in the list. */
    int addAreaType(std::string);

    /** @brief Adds the given boundary to the existing area type specified by the given ID . */
    bool setAreaBoundry(int, Coord, Coord);

    /** @brief Adds a time domain to the list with the given name  and returns the index of the this time domain in the list. */
    int addTimeDomain(std::string);

    /** @brief Adds the given boundary to the existing time domain specified by the given ID . */
    bool setTimeBoundry(int, simtime_t, simtime_t);

    /** @brief Adds a space-time combination to the list. */
    bool addCombination(std::string, std::string, std::string);

    /** @brief Gets a time and location, and returns the corresponding Markov transition matrix. */
    double **getMatrix(simtime_t, Coord);
};

} // namespace inet

#endif // ifndef __INET_POSTURETRANSITION_H

