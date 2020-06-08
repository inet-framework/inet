//
// Copyright (C) 2005 Georg Lutz, Institut fuer Telematik, University of Karlsruhe
// Copyright (C) 2005 Andras Varga
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

#include "inet/mobility/single/RandomWaypointMobility2.h"

namespace inet {

Define_Module(RandomWaypointMobility2);

RandomWaypointMobility2::RandomWaypointMobility2():RandomWaypointMobility()
{
}

void RandomWaypointMobility2::initialize(int stage)
{
    RandomWaypointMobility::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        if (strcmp(par("Active").stringValue(), "OFF") == 0)  {
            stationary = true;
        }
    }
}

} // namespace inet

