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

#include "mobility/StaticGridMobility.h"

Define_Module(StaticGridMobility);

/////////////////////////////// PUBLIC ///////////////////////////////////////

//============================= LIFECYCLE ===================================
/**
 * Initialization routine
 */
void StaticGridMobility::initialize(int aStage)
{
    BasicMobility::initialize(aStage);

    EV << "initializing StaticGridMobility stage " << aStage << endl;

    if (1 == aStage)
    {
        mNumHosts = par("numHosts");
        marginX = par("marginX");
        marginY = par("marginY");

        int size    = (int)ceil (sqrt(mNumHosts));
        double row  = ceil((hostPtr->getIndex()) / size);
        int col     = (hostPtr->getIndex()) % size;
        pos.x       = marginX + col * (getPlaygroundSizeX() - 2*marginX) / (size-1);
        if (pos.x >= getPlaygroundSizeX()) pos.x-=1;
        pos.y       = marginY + row * (getPlaygroundSizeY() - 2*marginY) / (size-1);
        if (pos.y >= getPlaygroundSizeY()) pos.y-=1;

        updatePosition();
    }

}

void StaticGridMobility::finish()
{
    BasicMobility::finish();
    recordScalar("x", pos.x);
    recordScalar("y", pos.y);
}
