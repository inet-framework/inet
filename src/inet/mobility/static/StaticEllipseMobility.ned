//
// Copyright (C) 2016 Kai Kientopf <kai.kientopf@uni-muenster.de>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

package inet.mobility.static;

import inet.mobility.base.MobilityBase;

//
// Places all hosts on a ellipse based on an given area over constraintAreaMin and constraintAreaMax.
// constraintAreaMax.x and constraintAreaMax.y should't be infinity.
// If constraintAreaMin.x or constraintAreaMin.y is infinity 0 is assumed.
//
simple StaticEllipseMobility extends MobilityBase
{
    parameters:
        int numHosts;
        double initialZ @unit(m) = default(0m);

        @class(StaticEllipseMobility);
}
