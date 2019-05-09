//
// Copyright (C) 2005 Georg Lutz, Institut fuer Telematik, University of Karlsruhe
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

package inet.mobility.single;

import inet.mobility.base.MovingMobilityBase;

//
// Implements the ManhattanGrid model.
//
//
// @author Alfonso Ariza
//
simple ManhattanGrid extends MovingMobilityBase
{
    parameters:
        double initialX @unit(m) = default(uniform(this.constraintAreaMinX, this.constraintAreaMaxX));
        double initialY @unit(m) = default(uniform(this.constraintAreaMinY, this.constraintAreaMaxY));
        double initialZ @unit(m) = default(nanToZero(uniform(this.constraintAreaMinZ, this.constraintAreaMaxZ)));
        int NumXstreets = default(1);
        int NumYstreets = default(1);
        bool initFromDisplayString = default(true); // enables one time initialization from the subject module's display string
        volatile double speed @unit(mps) = default(mps); // use uniform(minSpeed, maxSpeed) or another distribution
        volatile double waitTime @unit(s) = default(0s); // wait time between reaching a target and choosing a new one
        @class(ManhattanGrid);
}

