//
// Copyright (C) 2005 Georg Lutz, Institut fuer Telematik, University of Karlsruhe
// Copyright (C) 2005 Andras Varga
// Alfonso Ariza 2010, Univerdidad de Málaga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "RestrictedRandomWPMobility.h"
Define_Module(RestrictedRandomWPMobility);
void RestrictedRandomWPMobility::initialize(int stage)
{
	RandomWPMobility::initialize(stage);
    if (stage == 1)
     {
         x1 = par("x1");
         y1 = par("y1");
         x2 = par("x2");
         y2 = par("y2");

         if (x1<0)
         	x1=0;
         if (y1<0)
         	y1=0;
         if (x1>=x2)
         {
         	x1=0;
         	x2=getPlaygroundSizeX();
         }
         if (y1>=y2)
         {
         	y1=0;
         	y2=getPlaygroundSizeY();
         }
         if (x2>getPlaygroundSizeX())
         	x2=getPlaygroundSizeX();
         if (y2>getPlaygroundSizeY())
         	y2=getPlaygroundSizeY();
         if (pos.x<x1 || pos.x>x2 || pos.y<y1 || pos.y>y2)
         {
         	pos = getRandomPosition();
         	updatePosition();
          	setTargetPosition();
         }
         if (targetPos.x<x1 || targetPos.x>x2 || targetPos.y<y1 || targetPos.y>y2)
         {
          	setTargetPosition();
         }
     }
}

Coord RestrictedRandomWPMobility::getRandomPosition()
{
    Coord p;
    p.x = uniform(x1,x2);
    p.y = uniform(y1,y2);
    return p;
}
