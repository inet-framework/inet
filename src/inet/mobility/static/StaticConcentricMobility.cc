/*
 * Copyright (C) 2014 Florian Meier <florian.meier@koalo.de>
 *
 * Based on:
 * Copyright (C) 2006 Isabel Dietrich <isabel.dietrich@informatik.uni-erlangen.de>
 * Copyright (C) 2013 OpenSim Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "StaticConcentricMobility.h"

namespace inet {

Define_Module(StaticConcentricMobility);

int circleOfIndex(int idx) {
    // Returns the inverse of f(x)=sum_{r=1}^x floor(2*pi*r)
    // f(x) is the number of nodes on x concentric circles
    if(idx > 0) {
        return (1+sqrt(1+(4.0*idx)/M_PI))/2;
    }
    else {
        return 0;
    }
}

int nodesOnCircles(int circles) {
    if(circles < 0) {
        return 0;
    }
    else {
        int nodes = 1;
        for(int r = 0; r <= circles; r++) {
            nodes += (int)(2*M_PI*r);
        }
        return nodes;
    }
}

void StaticConcentricMobility::setInitialPosition()
{
    int numHosts = par("numHosts");
    double distance = par("distance");

    int index = visualRepresentation->getIndex();

    unsigned int myCircle = circleOfIndex(index);
    unsigned int totalCircles = circleOfIndex(numHosts-1); // -1 for center node
    unsigned int nodesOnInnerCircles = nodesOnCircles(myCircle-1);

    lastPosition.x = distance*totalCircles;
    lastPosition.y = distance*totalCircles;

    if(constraintAreaMin.x > -INFINITY) {
        lastPosition.x += constraintAreaMin.x;
    }

    if(constraintAreaMin.y > -INFINITY) {
        lastPosition.y += constraintAreaMin.y;
    }

    if(index > 0) {
        double radius = distance*myCircle;
        double angularStep = 2.0*M_PI/(int)(2*M_PI*myCircle);
        double angle = angularStep*(index-nodesOnInnerCircles);
        lastPosition.x += radius*cos(angle);
        lastPosition.y += radius*sin(angle);
    }

    lastPosition.z = par("initialZ");
    recordScalar("x", lastPosition.x);
    recordScalar("y", lastPosition.y);
    recordScalar("z", lastPosition.z);
}

} // namespace inet

