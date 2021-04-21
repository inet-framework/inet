/* -*- mode:c++ -*- ********************************************************
 * file:        PostureTransition.cc
 *
 * author:      Majid Nabi <m.nabi@tue.nl>
 *
 *              http://www.es.ele.tue.nl/nes
 *
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

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "inet/common/INETMath.h"
#include "inet/mobility/group/PostureTransition.h"

namespace inet {

/**
 * Constructor function of the class. It sets the value for t he number of posture. It also suppose the first given transition matrix as default.
 * However, during parsing the xml configuration file, if a matrix has attribute type with value "default", it will be considered as the default (base)
 * transition matrix.
 */
PostureTransition::PostureTransition(int numPosture)
{
    numPos = numPosture;
    defaultMatrixID = 0;    // if no default matrix found, the first one will be supposed as the default matrix.
}

PostureTransition::~PostureTransition()
{
    for (auto comb: combinationList) {
        delete comb;
    }
    for (auto mat: matrixList) {
        for (int i = 0; i < numPos; ++i)
            delete [] mat->matrix[i];
        delete [] mat->matrix;
        delete mat;
    }
    for (auto areaType: areaTypeList) {
        for (auto bound: areaType->boundries)
            delete bound;
        delete areaType;
    }
    for (auto timeDomain: timeDomainList) {
        for (auto bound: timeDomain->boundries)
            delete bound;
        delete timeDomain;
    }
}


/**
 * This function initiates a new instance of markov matrix with the given matrix. Note that it copies the matrix into the created matrix.
 * The function first verifies if the given matrix can be a Markov transition matrix.
 */
int PostureTransition::addMatrix(std::string name, double **matrix, bool thisDefault)
{
    //check if the name is repetitive
    TransMatrixList::const_iterator matrixIt;
    for (matrixIt = matrixList.begin(); matrixIt != matrixList.end(); matrixIt++) {
        if ((*matrixIt)->name == name) {
            std::string str = "There are multiple matrices with the same name: " + name + " in the configuration file!";
            throw cRuntimeError("%s", str.c_str());
        }
    }

    // verify if the given matrix is Markovian
    if (!isMarkovian(matrix)) {
        std::string str = "Given transition matrix " + name + " is not Markovian!";
        throw cRuntimeError("%s", str.c_str());
    }

    TransMatrix *mat = new TransMatrix;

    mat->name = name;
    mat->matrix = new double *[numPos];
    for (int i = 0; i < numPos; ++i) {
        mat->matrix[i] = new double[numPos];
        for (int j = 0; j < numPos; ++j)
            mat->matrix[i][j] = matrix[i][j];
    }

    matrixList.push_back(mat);

    if (thisDefault)
        defaultMatrixID = matrixList.size() - 1;

    return 0;
}

/**
 * This function creates a new instance of markov matrix to be filled with a derived matrix from the given steady state vector.
 * The function first verifies if the given vector can be a steady state vector. Then extracts a markov matrix based on that and adds
 * it to the list of given matrices.
 */
int PostureTransition::addSteadyState(std::string name, double *iVector)
{
    //check if the name is repetitive
    TransMatrixList::const_iterator matrixIt;
    for (matrixIt = matrixList.begin(); matrixIt != matrixList.end(); matrixIt++) {
        if ((*matrixIt)->name == name) {
            std::string str = "There are multiple matrices with the same name: " + name + " in the configuration file!";
            throw cRuntimeError("%s", str.c_str());
        }
    }

    // check if the given matrix is Markovian
    if (!isMarkovian(iVector)) {
        std::string str = "Given steady state vector " + name + " cannot be true!";
        throw cRuntimeError("%s", str.c_str());
    }

    // make a local copy of the input steady state vector
    double *steady = new double[numPos];
    for (int i = 0; i < numPos; ++i)
        steady[i] = iVector[i];

    TransMatrix *mat = new TransMatrix;
    mat->name = name;
    mat->matrix = extractMatrixFromSteadyState(steady);
    delete [] steady;

    matrixList.push_back(mat);

    return 0;
}

/**
 * Creates a new area type instance and adds it to the list. The boundaries of the area type is empty now. It will be filled later. The function returns
 * the index of the new area type in the list as its output.
 */
int PostureTransition::addAreaType(std::string name)
{
    //Check if the name is repetitive
    AreaTypeList::const_iterator areaIt;
    for (areaIt = areaTypeList.begin(); areaIt != areaTypeList.end(); areaIt++) {
        if ((*areaIt)->name == name) {
            std::string str = "There are multiple area types with the same name: " + name + " in the configuration file!";
            throw cRuntimeError("%s", str.c_str());
        }
    }

    AreaType *area = new AreaType;
    area->name = name;
    areaTypeList.push_back(area);
    return areaTypeList.size() - 1;
}

/**
 * This function gets an index of an existing area type and adds the given boundary to the boundary list of that area type.
 */
bool PostureTransition::setAreaBoundry(int id, Coord lowBound, Coord highBound)
{
    AreaBound *bound = new AreaBound;
    bound->low = lowBound;
    bound->high = highBound;

    areaTypeList.at(id)->boundries.push_back(bound);

    return true;
}

/**
 * Creates a new time domain instance and adds it to the list. The boundaries of the time domain is empty now. It will be filled later. The function returns
 * the index of the time domain in the list as its output.
 */
int PostureTransition::addTimeDomain(std::string name)
{
    //Check if the name is repetitive
    TimeDomainList::const_iterator timeIt;
    for (timeIt = timeDomainList.begin(); timeIt != timeDomainList.end(); timeIt++) {
        if ((*timeIt)->name == name) {
            std::string str = "There are multiple time domains with the same name: " + name + " in the configuration file!";
            throw cRuntimeError("%s", str.c_str());
        }
    }

    TimeDomainType *time = new TimeDomainType;
    time->name = name;
    timeDomainList.push_back(time);
    return timeDomainList.size() - 1;
}

/**
 * This function gets an index of an existing time domain and adds the given boundary to the boundary list of that time domain.
 */
bool PostureTransition::setTimeBoundry(int id, simtime_t lowBound, simtime_t highBound)
{
    TimeBound *bound = new TimeBound;
    bound->low = lowBound;
    bound->high = highBound;

    timeDomainList.at(id)->boundries.push_back(bound);

    return true;
}

/**
 * This function creates a new space-time combination instance and adds it to the combinations list. It checks if the given names for area type,
 * time domain, and matrix are previously defined and exist in the corresponding lists. Note that at least area type or time domain should have
 * been specified for a combination. Otherwise the combination is not meaningful. if for example a combination has no area type and just has specified
 * time domain, it means that for the whole simulation area, it will be the same and the proper matrix is selected based on the time.
 */
bool PostureTransition::addCombination(std::string areaName, std::string timeName, std::string matrixName)
{
    int thisID;
    CombinationType *comb = new CombinationType;
    comb->areaID = -1;
    comb->timeID = -1;
    comb->matrixID = -1;

    // look for matching area type name.
    thisID = 0;
    AreaTypeList::const_iterator areaIt;
    for (areaIt = areaTypeList.begin(); areaIt != areaTypeList.end(); areaIt++) {
        if (areaName == (*areaIt)->name) {
            comb->areaID = thisID;
            break;
        }
        ++thisID;
    }

    // in the input name is empty, it means that no area type is specified for this combination.
    if (comb->areaID == -1 && !areaName.empty()) {
        std::string str = "Undefined area type name is given in a combinations: " + areaName + ", " + timeName + ", " + matrixName;
        throw cRuntimeError("%s", str.c_str());
    }

    // look for matching time domain name.
    thisID = 0;
    TimeDomainList::const_iterator timeIt;
    for (timeIt = timeDomainList.begin(); timeIt != timeDomainList.end(); timeIt++) {
        if (timeName == (*timeIt)->name) {
            comb->timeID = thisID;
            break;
        }
        ++thisID;
    }
    if (comb->timeID == -1 && !timeName.empty()) {
        std::string str = "Undefined time domain name is given in a combinations: " + areaName + ", " + timeName + ", " + matrixName;
        throw cRuntimeError("%s", str.c_str());
    }

    if (comb->areaID == -1 && comb->timeID == -1)
        throw cRuntimeError("Both area type and time domain is unspecified in a combination.");

    // look for matching transition matrix name.
    thisID = 0;
    TransMatrixList::const_iterator matrixIt;
    for (matrixIt = matrixList.begin(); matrixIt != matrixList.end(); matrixIt++) {
        if (matrixName == (*matrixIt)->name) {
            comb->matrixID = thisID;
            break;
        }
        ++thisID;
    }
    if (comb->matrixID == -1)
        throw cRuntimeError("Undefined matrix name is given in the combinations");

    combinationList.push_back(comb);

    return true;
}

/**
 * This function is actually the main usage of this class. It gets a time instance and a location within the simulation area, and then
 * looks for the first fitting combination. If found, it returns the specified Markov transition matrix for that combination as its output.
 * If no combination is found, it returns the default matrix.
 */
double **PostureTransition::getMatrix(simtime_t iTime, Coord iLocation)
{
    int timeID, locationID, matrixID;

    timeID = findTimeDomain(iTime);
    locationID = findAreaType(iLocation);

    matrixID = defaultMatrixID;

    CombinationList::const_iterator combIt;
    for (combIt = combinationList.begin(); combIt != combinationList.end(); combIt++) {
        if ((*combIt)->timeID == timeID && (*combIt)->areaID == locationID) {
            matrixID = (*combIt)->matrixID;
            break;
        }
    }

    EV_DEBUG << "The corresponding Markov matrix for time" << iTime.dbl() << " and location " << iLocation.str() << " is: " << matrixList.at(matrixID)->name << endl;

    return matrixList.at(matrixID)->matrix;
}

/**
 * Looks for the first containing time domain for the given time instance. It return the Id of the found time domain. If no time domain
 * is found which contains the given time instance, it returns -1.
 */
int PostureTransition::findTimeDomain(simtime_t iTime)
{
    int timeID = 0;
    TimeDomainList::const_iterator timeIt;
    for (timeIt = timeDomainList.begin(); timeIt != timeDomainList.end(); timeIt++) {
        std::vector<TimeBound *> boundList = (*timeIt)->boundries;

        std::vector<TimeBound *>::const_iterator bound;
        for (bound = boundList.begin(); bound != boundList.end(); bound++) {
            if (iTime >= (*bound)->low && iTime < (*bound)->high)
                return timeID;
        }
        ++timeID;
    }
    EV_DEBUG << "Time domain not found" << endl;
    return -1;
}

/**
 * Looks for the first containing area type for the given location. It return the Id of the found area type. If no area type
 * is found which contains the given location, it returns -1.
 */
int PostureTransition::findAreaType(Coord iLocation)
{
    int locationID = 0;
    AreaTypeList::const_iterator areaIt;
    for (areaIt = areaTypeList.begin(); areaIt != areaTypeList.end(); areaIt++) {
        std::vector<AreaBound *> boundList = (*areaIt)->boundries;

        std::vector<AreaBound *>::const_iterator bound;
        for (bound = boundList.begin(); bound != boundList.end(); bound++) {
            if (iLocation.isInBoundary((*bound)->low, (*bound)->high))
                return locationID;
        }
        ++locationID;
    }
    EV_DEBUG << "Area Type not found" << endl;
    return -1;
}

/**
 * Verifies if a matrix can be a Markovian transition matrix. Each element of the matrix should be in the range [0 1].
 * Further, all elements of each column should adds up to one.
 */
bool PostureTransition::isMarkovian(double **matrix)
{
    double sumCol;
    for (int j = 0; j < numPos; ++j) {
        sumCol = 0;
        for (int i = 0; i < numPos; ++i) {
            if (matrix[i][j] < 0 || matrix[i][j] > 1)
                return false;
            sumCol += matrix[i][j];
        }

        if (!math::close(sumCol, 1.0))
            return false;
    }

    return true;
}

/**
 * Verifies if a vector can be the steady state of a Markov model. Each element of the matrix should be in the range [0 1].
 * Further, the sum of all elements should be one.
 */
bool PostureTransition::isMarkovian(double *vec)
{
    double sumCol = 0;
    for (int i = 0; i < numPos; ++i) {
        if (vec[i] < 0 || vec[i] > 1)
            return false;
        sumCol += vec[i];
    }

    if (!math::close(sumCol, 1.0))
        return false;
    else
        return true;
}

/**
 * Function to multiply two matrix with the known dimensions as number of postures.
 */
void PostureTransition::multMatrix(double **mat1, double **mat2, double **res)
{
    int i, j, l;
    for (i = 0; i < numPos; i++) {
        for (j = 0; j < numPos; j++) {
            res[i][j] = 0;
            for (l = 0; l < numPos; l++)
                res[i][j] += mat1[i][l] * mat2[l][j];
        }
    }
}

/**
 * Function to add two matrix with the known dimensions as number of postures.
 */
void PostureTransition::addMatrix(double **mat1, double **mat2, double **res)
{
    int i, j;
    for (i = 0; i < numPos; i++) {
        for (j = 0; j < numPos; j++)
            res[i][j] = mat1[i][j] + mat2[i][j];
    }
}

/**
 * Function to subtract two matrix with the known dimensions as number of postures.
 */
void PostureTransition::subtractMatrix(double **mat1, double **mat2, double **res)
{
    int i, j;
    for (i = 0; i < numPos; i++) {
        for (j = 0; j < numPos; j++)
            res[i][j] = mat1[i][j] - mat2[i][j];
    }
}

/**
 * Function to multiply a vector by its transpose (pi . pi^T). The size in equal to the number of postures.
 */
void PostureTransition::multVector(double *vec, double **res)
{
    int i, j;
    for (i = 0; i < numPos; i++) {
        for (j = 0; j < numPos; j++)
            res[i][j] = vec[i] * vec[j];
    }
}

/**
 * This function receives a steady state vector and extracts a Markovian matrix which is as close as possible to the default markov matrix and
 * satisfies the given steady state vector.
 */
double **PostureTransition::extractMatrixFromSteadyState(double *vec)
{
    int i, j;
    double **dafaultMat;

    //make output matrix and an identity matrix and a temp
    double **mat = new double *[numPos];
    double **temp1 = new double *[numPos];
    double **temp2 = new double *[numPos];
    double **temp3 = new double *[numPos];
    double **identity = new double *[numPos];
    int **change = new int *[numPos];
    for (int i = 0; i < numPos; ++i) {
        mat[i] = new double[numPos];
        temp1[i] = new double[numPos];
        temp2[i] = new double[numPos];
        temp3[i] = new double[numPos];
        identity[i] = new double[numPos];
        change[i] = new int[numPos];
    }

    for (i = 0; i < numPos; i++)
        for (j = 0; j < numPos; j++)
            if (i == j)
                identity[i][j] = 1;
            else
                identity[i][j] = 0;

    double *sum = new double[numPos];
    int *changeSum = new int[numPos];

    dafaultMat = matrixList.at(defaultMatrixID)->matrix;

    for (int numTry = 0; numTry < 400; ++numTry) {
        subtractMatrix(identity, dafaultMat, temp1);
        multVector(vec, temp2);
        multMatrix(temp1, temp2, temp3);
        addMatrix(dafaultMat, temp3, mat);

        //remember if it has not changed
        for (i = 0; i < numPos; i++)
            for (j = 0; j < numPos; j++)
                change[i][j] = 1;


        for (j = 0; j < numPos; j++)
            for (i = 0; i < numPos; i++) {
                if (mat[i][j] < 0) {
                    mat[i][j] = 0;
                    change[i][j] = 0;
                }
                if (mat[i][j] > 1) {
                    mat[i][j] = 1;
                    change[i][j] = 0;
                }
            }

        for (j = 0; j < numPos; j++) {
            sum[j] = 0;
            changeSum[j] = 0;
            for (i = 0; i < numPos; i++) {
                sum[j] += mat[i][j];
                changeSum[j] += change[i][j];
            }
        }

        for (j = 0; j < numPos; j++)
            for (i = 0; i < numPos; i++) {
                if (change[i][j] == 1)
                    mat[i][j] = mat[i][j] + (1 - sum[j]) / changeSum[j];
            }

        dafaultMat = mat;
    }

    for (j = 0; j < numPos; j++)
        for (i = 0; i < numPos; i++) {
            if (mat[i][j] < 0)
                mat[i][j] = 0;
            if (mat[i][j] > 1)
                mat[i][j] = 1;
        }

    EV_DEBUG << "Generated Markov matrix from the steady state: " << endl;
    for (int k = 0; k < numPos; ++k) {
        for (int f = 0; f < numPos; ++f)
            EV_DEBUG << mat[k][f] << "       ";
        EV_DEBUG << endl;
    }

    for (int i = 0; i < numPos; ++i) {
        delete [] temp1[i];
        delete [] temp2[i];
        delete [] temp3[i];
        delete [] identity[i];
        delete [] change[i];
    }
    delete [] temp1;
    delete [] temp2;
    delete [] temp3;
    delete [] identity;
    delete [] change;
    delete [] sum;
    delete [] changeSum;

    return mat;
}

} // namespace inet

