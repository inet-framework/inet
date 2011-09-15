/*
 * Copyright (C) 2006 Isabel Dietrich <isabel.dietrich@informatik.uni-erlangen.de>
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


#include "StaticGridMobility.h"


Define_Module(StaticGridMobility);


StaticGridMobility::StaticGridMobility()
{
    marginX = 0;
    marginY = 0;
    numHosts = 0;
}

void StaticGridMobility::initialize(int stage)
{
    MobilityBase::initialize(stage);
    EV << "initializing StaticGridMobility stage " << stage << endl;
    if (stage == 0)
    {
        numHosts = par("numHosts");
        marginX = par("marginX");
        marginY = par("marginY");
    }
}

void StaticGridMobility::initializePosition()
{
    int index = visualRepresentation->getIndex();
    int size = (int)ceil(sqrt((double)numHosts));
    int row = (int)floor((double)index / (double)size);
    int col = index % size;
    lastPosition.x = constraintAreaMin.x + marginX
            + col * ((constraintAreaMax.x - constraintAreaMin.x) - 2 * marginX) / (size - 1);
    if (lastPosition.x >= constraintAreaMax.x)
        lastPosition.x -= 1;
    lastPosition.y = constraintAreaMin.y + marginY
            + row * ((constraintAreaMax.y - constraintAreaMin.y) - 2 * marginY) / (size - 1);
    if (lastPosition.y >= constraintAreaMax.y)
        lastPosition.y -= 1;
    lastPosition.z = 0;
}

void StaticGridMobility::finish()
{
    MobilityBase::finish();
    recordScalar("x", lastPosition.x);
    recordScalar("y", lastPosition.y);
}
