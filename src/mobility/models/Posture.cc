/* -*- mode:c++ -*- ********************************************************
 * file:        Posture.cc
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
 * description:     A class to store the specification of a posture
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



#include "Posture.h"

Posture::Posture(unsigned int ID, unsigned int num)
{
    postureID = ID;
    numNodes = num;

    nodePs = new Coord[numNodes];
    nodeRadius = new double[numNodes];
    nodeSpeed =  new double[numNodes];

    alphaMean = new double*[numNodes];
    for (unsigned int i=0;i<numNodes;++i)
        alphaMean[i] = new double[numNodes];

    alphaSD = new double*[numNodes];
    for (unsigned int i=0;i<numNodes;++i)
        alphaSD[i] = new double[numNodes];
}

bool Posture::setPs(unsigned int i , Coord ps)
{
    if (i<numNodes){
        nodePs[i] = ps;
        return true;
    }
    return false;
}

bool Posture::setPostureName(char *str)
{
    strcpy(posture_name,str);
    return true;
}


bool Posture::setAlphaMean(unsigned int i, unsigned int j, double alpha_mean)
{
    if (i<numNodes && j<numNodes){
        alphaMean[i][j] = alpha_mean;
        return true;
    }
    return false;
}

bool Posture::setAlphaSD(unsigned int i, unsigned int j, double alpha_sd)
{
    if (i<numNodes && j<numNodes){
        alphaSD[i][j] = alpha_sd;
        return true;
    }
    return false;
}

bool Posture::setRadius(unsigned int i, double radius)
{
    if (i<numNodes){
        nodeRadius[i] = radius;
        return true;
    }
    return false;
}

bool Posture::setSpeed(unsigned int i , double speed)
{
    if (i<numNodes){
        nodeSpeed[i] = speed;
        return true;
    }
    return false;
}

bool Posture::setPostureSpeed(double min, double max)
{
    if (max < min)
        return false;

    maxSpeed = max;
    minSpeed = min;
    return true;
}

Coord  Posture::getPs(unsigned int i)
{
    if (i<numNodes)
        return nodePs[i];

    return Coord(-1,-1,-1);
}

double Posture::getAlphaMean(unsigned int i, unsigned int j)
{
    if (i<numNodes && j<numNodes)
        return alphaMean[i][j];

    return -1;
}

double Posture::getAlphaSD(unsigned int i, unsigned int j)
{
    if (i<numNodes && j<numNodes)
        return alphaSD[i][j];

    return -1;
}

double Posture::getRadius(unsigned int i)
{
    if (i<numNodes)
        return nodeRadius[i];

    return -1;
}

double Posture::getSpeed(unsigned int i)
{
    if (i<numNodes)
        return nodeSpeed[i];

    return -1;
}

char* Posture::getPostureName()
{
    return posture_name;
}

int Posture::getPostureID()
{
    return postureID;
}

double Posture::getMaxSpeed()
{
    return maxSpeed;
}

double Posture::getMinSpeed()
{
    return minSpeed;
}

bool Posture::isMobile()
{
    return maxSpeed > 0;
}
