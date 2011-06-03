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

        int size = (int)ceil(sqrt((double)mNumHosts));
        double row = ceil((hostPtr->getIndex()) / (double)size);
        int col = (hostPtr->getIndex()) % size;

        pos.x = areaTopLeft.x + marginX
                + col * ((areaBottomRight.x - areaTopLeft.x) - 2 * marginX) / (size - 1);

        if (pos.x >= areaBottomRight.x)
            pos.x -= 1;

        pos.y = areaTopLeft.y + marginY
                + row * ((areaBottomRight.y - areaTopLeft.y) - 2 * marginY) / (size - 1);

        if (pos.y >= areaBottomRight.y)
            pos.y -= 1;

        positionUpdated();
    }

}

void StaticGridMobility::finish()
{
    BasicMobility::finish();
    recordScalar("x", pos.x);
    recordScalar("y", pos.y);
}
